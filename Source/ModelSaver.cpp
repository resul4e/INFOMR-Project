#include "ModelSaver.h"

#include "ModelDescriptor.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>
#include <assimp/cexport.h>

#include <limits>
#include <iostream>
#include <fstream>
#include <glm/gtx/string_cast.hpp>

namespace fs = std::filesystem;

void ModelSaver::SavePly(ModelDescriptor& _modelDescriptor, fs::path _filePath)
{
	const Model& model = *_modelDescriptor.m_model;

	//Create the exporter and the scene
	Assimp::Exporter exporter;
	aiScene* scene = new aiScene();

	//Create a mesh for each mesh that we have in our model.
	aiMesh* meshes = new aiMesh[model.m_meshes.size()];

	//Extract the positions and faces from each mesh and add them to the assimp scene.
	for(int i = 0; i < model.m_meshes.size(); i++)
	{
		meshes[i].mNumVertices = model.m_meshes[i].positions.size();
		meshes[i].mVertices = new aiVector3D[meshes[i].mNumVertices];

		for(int v = 0; v < meshes[i].mNumVertices; v++)
		{
			meshes[i].mVertices[v] = aiVector3D{ model.m_meshes[i].positions[v].x, model.m_meshes[i].positions[v].y , model.m_meshes[i].positions[v].z };
		}

		meshes[i].mNumFaces = model.m_meshes[i].faces.size();
		meshes[i].mFaces = new aiFace[meshes[i].mNumFaces];
		for(int f = 0; f < model.m_meshes[i].faces.size(); f++)
		{
			meshes[i].mFaces[f].mNumIndices = 3;
			meshes[i].mFaces[f].mIndices = new unsigned int[3];
			for(int j = 0; j < 3; j++)
			{
				meshes[i].mFaces[f].mIndices[j] = model.m_meshes[i].faces[f].indices[j];
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

	scene->mNumMeshes = model.m_meshes.size();
	scene->mMeshes = &meshes;

	//Save the scene.
	exporter.Export(scene, exporter.GetExportFormatDescription(7)->id, _filePath.string());

	//Change the filepath to be the new path.
	_modelDescriptor.m_path = _filePath;

	SaveFeatures(_modelDescriptor);
}

void ModelSaver::SaveFeatures(ModelDescriptor& _modelDescriptor)
{
	const fs::path featuresDatabasePath("../FeatureDatabase");
	fs::create_directory(featuresDatabasePath);

	fs::path featuresPath = featuresDatabasePath / _modelDescriptor.m_path.filename().replace_extension(".csv");

	std::ofstream featuresStream(featuresPath.string());
	if(!featuresStream.is_open())
	{
		std::cerr << "Could not save " << featuresPath;
		return;
	}

	Features3D features = _modelDescriptor.m_3DFeatures;
	
	featuresStream << "volume, " << features.volume << "\n";
	featuresStream << "surfaceArea, " << features.surfaceArea << "\n";
	featuresStream << "compactness, " << features.compactness << "\n";
	featuresStream << "boundsArea, " << features.boundsArea << "\n";
	featuresStream << "boundsVolume, " << features.boundsVolume << "\n";
	featuresStream << "eccentricity, " << features.eccentricity << "\n";

	featuresStream << "a3, ";
	SaveHistogramFeatures(features.a3, featuresStream);

	featuresStream << "d1, ";
	SaveHistogramFeatures(features.d1, featuresStream);

	featuresStream << "d2, ";
	SaveHistogramFeatures(features.d2, featuresStream);

	featuresStream << "d3, ";
	SaveHistogramFeatures(features.d3, featuresStream);

	featuresStream << "d4, ";
	SaveHistogramFeatures(features.d4, featuresStream);
	
	featuresStream.close();
}

void ModelSaver::SaveHistogramFeatures(HistogramFeature _feature, std::ofstream& _stream)
{
	_stream << _feature.m_numBins << " ";
	for(int i = 0; i < _feature.m_numBins; i++)
	{
		_stream << _feature[i] << ", ";
	}
	_stream << _feature.m_min << ", " << _feature.m_max;
	_stream << "\n";
}
