#include "Model.h"

#include <iostream>

Mesh::Mesh()
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
	m_class("")
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
