#include "Model.h"

#include <pmp/algorithms/SurfaceNormals.h>

#include <iostream>
#include "FeatureExtraction.h"
#include "ModelUtil.h"

Mesh::Mesh() :
	vao(0),
	pbo(0),
	nbo(0),
	tbo(0)
{

}
#include <QDebug>
void Mesh::Upload()
{
	QOpenGLFunctions_3_3_Core* f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
	qDebug() << "Current: " << QOpenGLContext::currentContext();
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

			if (texCoords.size() != 0 && texCoords.size() > face.indices[v]) {
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
	m_isUploaded(false)
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

void Model::markForReupload()
{
	m_isUploaded = false;
}

void Model::CalculateOBB()
{
	
	m_orientedPoints.clear();

	glm::vec3 eigenVectors[3];
	glm::vec3 eigenValues;
	util::ComputeEigenVectors(*this, eigenVectors[0], eigenVectors[1], eigenVectors[2], eigenValues);

	// project min and max points on each principal axis
	float min1 = INFINITY, max1 = -INFINITY;
	float min2 = INFINITY, max2 = -INFINITY;
	float min3 = INFINITY, max3 = -INFINITY;
	float d = 0.0;

	for (const Mesh& mesh : m_meshes)
	{
		for (const glm::vec3& p : mesh.positions)
		{
			d = glm::dot(eigenVectors[0], p);
			if (min1 > d) min1 = d;
			if (max1 < d) max1 = d;

			d = glm::dot(eigenVectors[1], p);
			if (min2 > d) min2 = d;
			if (max2 < d) max2 = d;

			d = glm::dot(eigenVectors[2], p);
			if (min3 > d) min3 = d;
			if (max3 < d) max3 = d;
		}
	}
	
	// add points to vector
	m_orientedPoints.push_back(eigenVectors[0] * min1);
	m_orientedPoints.push_back(eigenVectors[0] * max1);
	m_orientedPoints.push_back(eigenVectors[1] * min2);
	m_orientedPoints.push_back(eigenVectors[1] * max2);
	m_orientedPoints.push_back(eigenVectors[2] * min3);
	m_orientedPoints.push_back(eigenVectors[2] * max3);
}
