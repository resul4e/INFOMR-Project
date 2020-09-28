#include "ModelLoader.h"

#include "Model.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <limits>
#include <iostream>

// Convert an Assimp matrix to a GLM column-major matrix
glm::mat4 convertMatrix(const aiMatrix4x4& aiMat)
{
	return {
		aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
		aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
		aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
		aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
	};
}

std::shared_ptr<Model> ModelLoader::LoadModel(std::filesystem::path _filePath)
{
	Assimp::Importer importer;

	unsigned int flags = aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_SortByPType;
	const aiScene* scene = importer.ReadFile(_filePath.string(), flags);

	// If the scene is null then the file has failed to load properly for some reason
	if (!scene)
	{
		std::cerr << "Failed to load model: " << importer.GetErrorString() << std::endl;
		return nullptr;
	}

	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->m_path = _filePath;
	model->m_meshes.resize(scene->mNumMeshes);
	model->m_name = _filePath.filename().replace_extension("").string();
	
	// Initial minimum and maximum bounds of the model
	glm::vec3 minBounds = glm::vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	glm::vec3 maxBounds = glm::vec3(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* aiMesh = scene->mMeshes[i];

		Mesh& mesh = model->m_meshes[i];

		// Copy vertex positions from assimp mesh to our mesh
		if (aiMesh->HasPositions())
		{
			mesh.positions.resize(aiMesh->mNumVertices);
			model->m_vertexCount += aiMesh->mNumVertices;
			std::memcpy(mesh.positions.data(), aiMesh->mVertices, aiMesh->mNumVertices * sizeof(glm::vec3));
		}
		
		// Copy vertex texture coordinates from assimp mesh to our mesh
		if (aiMesh->HasTextureCoords(0))
		{
			mesh.texCoords.resize(aiMesh->mNumVertices);
			std::memcpy(mesh.texCoords.data(), aiMesh->mTextureCoords[0], aiMesh->mNumVertices * sizeof(glm::vec2));

			mesh.texCoords.resize(aiMesh->mNumVertices);
			// We want to copy the 0th element from every vertex, since this is strided memory we can't use memcpy
			for (unsigned int j = 0; j < aiMesh->mNumVertices; j++) {
				mesh.texCoords[j] = glm::vec2(aiMesh->mTextureCoords[0][j].x, aiMesh->mTextureCoords[0][j].y);
			}
		}

		// Copy vertex normals from assimp mesh to our mesh
		if (aiMesh->HasNormals())
		{
			mesh.normals.resize(aiMesh->mNumVertices);
			std::memcpy(mesh.normals.data(), aiMesh->mNormals, aiMesh->mNumVertices * sizeof(glm::vec3));
		}

		// Copy faces from assimp mesh to our mesh
		mesh.faces.resize(aiMesh->mNumFaces);
		model->m_faceCount += aiMesh->mNumFaces;
		for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
		{
			for (unsigned int v = 0; v < 3; v++)
				mesh.faces[i].indices[v] = aiMesh->mFaces[i].mIndices[v];
		}

		// Compute model bounds
		for (int i = 0; i < aiMesh->mNumVertices; i++)
		{
			aiVector3D& v = aiMesh->mVertices[i];
		}
	}

	model->UpdateFeatures();
	return model;
}