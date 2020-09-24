#include "ModelSaver.h"

#include "Model.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>
#include <assimp/cexport.h>

#include <limits>
#include <iostream>

void ModelSaver::SavePly(Model& _model, std::filesystem::path _filePath)
{
	//Create the exporter and the scene
	Assimp::Exporter exporter;
	aiScene* scene = new aiScene();

	//Create a mesh for each mesh that we have in our model.
	aiMesh* meshes = new aiMesh[_model.m_meshes.size()];

	//Extract the positions and faces from each mesh and add them to the assimp scene.
	for(int i = 0; i < _model.m_meshes.size(); i++)
	{
		meshes[i].mNumVertices = _model.m_meshes[i].positions.size();
		meshes[i].mVertices = new aiVector3D[meshes[i].mNumVertices];

		for(int v = 0; v < meshes[i].mNumVertices; v++)
		{
			meshes[i].mVertices[v] = aiVector3D{ _model.m_meshes[i].positions[v].x,_model.m_meshes[i].positions[v].y ,_model.m_meshes[i].positions[v].z };
		}

		meshes[i].mNumFaces = _model.m_meshes[i].faces.size();
		meshes[i].mFaces = new aiFace[meshes[i].mNumFaces];
		for(int f = 0; f < _model.m_meshes[i].faces.size(); f++)
		{
			meshes[i].mFaces[f].mNumIndices = 3;
			meshes[i].mFaces[f].mIndices = new unsigned int[3];
			for(int j = 0; j < 3; j++)
			{
				meshes[i].mFaces[f].mIndices[j] = _model.m_meshes[i].faces[f].indices[j];
			}
		}

		scene->mNumMaterials = 1;
	}
	
	//We need empty materials for the file to be saved properly.
	aiMaterial* mats = new aiMaterial[scene->mNumMaterials];
	scene->mMaterials = &mats;

	//We need a root node for the application to not crash, and we need to associate the mesh we just added to the scene to this node.
	scene->mRootNode = new aiNode{"Root"};
	scene->mRootNode->mMeshes = new unsigned int[1];
	scene->mRootNode->mMeshes[0] = 0;
	scene->mRootNode->mNumMeshes = 1;

	scene->mNumMeshes = _model.m_meshes.size();
	scene->mMeshes = &meshes;

	//Save the scene.
	exporter.Export(scene, exporter.GetExportFormatDescription(7)->id, _filePath.string());

	//Change the filepath to be the new path.
	_model.m_path = _filePath;
}