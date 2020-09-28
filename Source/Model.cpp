#include "Model.h"

#include <pmp/algorithms/SurfaceNormals.h>

#include <iostream>
#include "FeatureExtraction.h"

# define M_PI           3.14159265358979323846  /* pi */

Mesh::Mesh() :
	vao(0),
	pbo(0),
	nbo(0),
	tbo(0)
{

}

void Mesh::Upload()
{
	QOpenGLFunctions_3_3_Core* f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();

	// Go through all faces and linearize the vertex data for uploading to the graphics card
	std::vector<glm::vec3> linearPositions(faces.size() * 3);
	std::vector<glm::vec3> linearNormals(faces.size() * 3);
	std::vector<glm::vec2> linearTextureCoords(faces.size() * 3);

	std::cout << faces.size() << std::endl;
	for (int i = 0; i < faces.size(); i++)
	{
		Face& face = faces[i];
		for (int v = 0; v < 3; v++)
		{
			linearPositions[i * 3 + v] = positions[face.indices[v]];
			linearNormals[i * 3 + v] = normals[face.indices[v]];

			if (texCoords.size() != 0) {
				linearTextureCoords[i * 3 + v] = texCoords[face.indices[v]];
			}
		}
	}

	// Generate the initial vertex array and buffer objects
	f->glGenVertexArrays(1, &vao);
	f->glBindVertexArray(vao);

	f->glGenBuffers(1, &pbo);
	f->glBindBuffer(GL_ARRAY_BUFFER, pbo);
	f->glBufferData(GL_ARRAY_BUFFER, linearPositions.size() * sizeof(glm::vec3), linearPositions.data(), GL_DYNAMIC_DRAW);
	f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	f->glEnableVertexAttribArray(0);

	f->glGenBuffers(1, &nbo);
	f->glBindBuffer(GL_ARRAY_BUFFER, nbo);
	f->glBufferData(GL_ARRAY_BUFFER, linearNormals.size() * sizeof(glm::vec3), linearNormals.data(), GL_DYNAMIC_DRAW);
	f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	f->glEnableVertexAttribArray(1);

	if (texCoords.size() != 0) {
		f->glGenBuffers(1, &tbo);
		f->glBindBuffer(GL_ARRAY_BUFFER, tbo);
		f->glBufferData(GL_ARRAY_BUFFER, linearTextureCoords.size() * sizeof(glm::vec2), linearTextureCoords.data(), GL_DYNAMIC_DRAW);
		f->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		f->glEnableVertexAttribArray(2);
	}
}

Model::Model() :
	m_isUploaded(false),
	m_class(""),
	m_vertexCount(0),
	m_faceCount(0)
{

}

void Model::Upload()
{
	for (Mesh& mesh : m_meshes)
	{
		mesh.Upload();
	}
	m_isUploaded = true;
}

bool Model::isUploaded()
{
	return m_isUploaded;
}

void Model::ToPmpModel(std::vector<pmp::SurfaceMesh>& pmpMeshes)
{
	pmpMeshes.resize(m_meshes.size());

	for (int i = 0; i < m_meshes.size(); i++)
	{
		const Mesh& mesh = m_meshes[i];
		pmp::SurfaceMesh& pmpMesh = pmpMeshes[i];

		for (const glm::vec3& v : mesh.positions)
		{
			pmpMesh.add_vertex(pmp::Point(v.x, v.y, v.z));
		}

		for (const Face& face : mesh.faces)
		{
			pmp::Vertex v0(face.indices[0]);
			pmp::Vertex v1(face.indices[1]);
			pmp::Vertex v2(face.indices[2]);

			pmpMesh.add_triangle(v0, v1, v2);
		}
	}
}

void Model::FromPmpModel(std::vector<pmp::SurfaceMesh>& pmpMeshes)
{
	m_vertexCount = 0;
	m_faceCount = 0;
	
	for (int i = 0; i < m_meshes.size(); i++)
	{
		pmp::SurfaceMesh& pmpMesh = pmpMeshes[i];
		Mesh& mesh = m_meshes[i];

		mesh.positions.resize(pmpMesh.n_vertices(), glm::vec3(0, 0, 0));
		m_vertexCount += pmpMesh.n_vertices();
		for (int j = 0; j < pmpMesh.positions().size(); j++)
		{
			mesh.positions[j] = glm::vec3(pmpMesh.positions()[j][0], pmpMesh.positions()[j][1], pmpMesh.positions()[j][2]);
		}

		mesh.normals.resize(pmpMesh.n_vertices(), glm::vec3(0, 1, 0));
		auto normals = pmpMesh.get_vertex_property<pmp::Point>("v:normal");
		for (auto vit : pmpMesh.vertices())
		{
			mesh.normals[vit.idx()] = glm::vec3(normals[vit][0], normals[vit][1], normals[vit][2]);
		}

		mesh.faces.clear();
		for (pmp::Face pmpFace : pmpMesh.faces())
		{
			m_faceCount++;
			Face face;
			int c = 0;
			for (pmp::Vertex v : pmpMesh.vertices(pmpFace))
			{
				face.indices[c++] = v.idx();
			}

			mesh.faces.push_back(face);
		}
	}

	m_isUploaded = false;
}

void Model::UpdateFeatures()
{
	UpdateBounds();

	m_3DFeatures.surfaceArea = ExtractSurfaceArea(*this);
	m_3DFeatures.volume = ExtractVolume(*this);
	m_3DFeatures.boundsArea = ExtractAABBArea(*this);
	m_3DFeatures.compactness = (m_3DFeatures.surfaceArea * m_3DFeatures.surfaceArea) / (4.0f * M_PI * m_3DFeatures.volume);
}

void Model::UpdateBounds()
{
	m_bounds.max = glm::vec3(0, 0, 0);
	m_bounds.min = glm::vec3(0, 0, 0);
	for(int k = 0; k < m_meshes.size(); k++)
	{
		for(int j = 0; j < m_meshes[k].positions.size(); j++)
		{
			glm::vec3 v = m_meshes[k].positions[j];
			for (int d = 0; d < 3; d++)
			{
				m_bounds.min[d] = v[d] < m_bounds.min[d] ? v[d] : m_bounds.min[d];
				m_bounds.max[d] = v[d] > m_bounds.max[d] ? v[d] : m_bounds.max[d];
			}
		}
	}

}
