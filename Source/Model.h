#pragma once

#include <QOpenGLFunctions_3_3_Core>

#include <glm/glm.hpp>

#include <vector>

struct Face
{
	unsigned int indices[3];
};

struct Bounds
{
	glm::vec3 min;
	glm::vec3 max;
};

struct Mesh
{
	Mesh();

	/**
	 * \brief Upload the mesh data to the graphics card video memory
	 * \remark Needs a valid OpenGL context to be bound when this function is called
	 */
	void Upload();

	/** Array of vertex positions belonging to this mesh */
	std::vector<glm::vec3> positions;
	/** Array of vertex texture coordinates belonging to this mesh */
	std::vector<glm::vec2> texCoords;
	/** Array of vertex normals belonging to this mesh */
	std::vector<glm::vec3> normals;
	/** Array of faces which index into the vertex arrays and form geometry */
	std::vector<Face> faces;

	// Graphics vertex buffer objects
	unsigned int vao;
	unsigned int pbo;
	unsigned int nbo;
	unsigned int tbo;
};

class Model
{
public:
	Model();

	/**
	 * \brief Upload the model data to the graphics card video memory
	 * \remark Needs a valid OpenGL context to be bound when this function is called
	 */
	void Upload();

	bool isUploaded();

	void markForReupload();

	std::vector<Mesh> m_meshes;

	std::vector<glm::vec3> m_orientedPoints;
	
private:
	void CalculateOBB();

	bool m_isUploaded;

	friend struct ModelDescriptor;
};
