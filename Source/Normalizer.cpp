#include "Normalizer.h"

#include "ModelUtil.h"

#include <iostream>
#include "ModelLoader.h"
#include "ModelSaver.h"
#include "ModelAnalytics.h"

#include "glm/gtx/component_wise.hpp"

namespace fs = std::filesystem;

void Normalizer::Normalize(ModelDescriptor& _modelDescriptor)
{
	AlignModel(*_modelDescriptor.m_model);
	CenterModel(*_modelDescriptor.m_model);
	FlipModel(*_modelDescriptor.m_model);
	ScaleModel(*_modelDescriptor.m_model);

	_modelDescriptor.UpdateBounds();
	_modelDescriptor.UpdateFeatures();
	_modelDescriptor.m_model->markForReupload();
}

void Normalizer::CenterModel(Model& _model)
{
	glm::vec3 mean = util::ComputeBarycenter(_model);

	// Subtract mean from each vertex position
	for (Mesh& mesh : _model.m_meshes)
		for (glm::vec3& p : mesh.positions)
			p -= mean;
}

void Normalizer::ScaleModel(Model& _model)
{
	// Find the maximum and minimum value for each of the three axes.
	glm::vec3 max{ -std::numeric_limits<float>::max() };
	glm::vec3 min{ std::numeric_limits<float>::max() };

	for (const Mesh& mesh : _model.m_meshes)
	{
		for (const glm::vec3& p : mesh.positions)
		{
			for (int k = 0; k < 3; k++)
			{
				max[k] = std::max(max[k], p[k]);
				min[k] = std::min(min[k], p[k]);
			}
		}
	}

	// Find the largest difference in one of the three axes.
	glm::vec3 range = max - min;
	float largestDiff = glm::compMax(range);

	// Divide each position by the largest difference.
	for (Mesh& mesh : _model.m_meshes)
	{
		for (glm::vec3& p : mesh.positions)
		{
			p /= largestDiff;
		}
	}
}

void Normalizer::AlignModel(Model& _model)
{
	util::RotateMajorEigenVectorToXAxis(_model);
}

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

void Normalizer::FlipModel(Model& _model)
{
	glm::vec3 f{ 0,0,0 };
	for (int l = 0; l < _model.m_meshes.size(); l++)
	{
		for (int j = 0; j < _model.m_meshes[l].faces.size(); j++)
		{
			glm::vec3 center{ 0,0,0 };
			for(int k = 0; k < 3; k++)
			{
				center += _model.m_meshes[l].positions[_model.m_meshes[l].faces[j].indices[k]];
			}
			center /= 3;

			for (int k = 0; k < 3; k++)
			{
				f += sgn(center[k]) * (center[k] * center[k]);
			}
		}
	}

	//If we are going to put all vertices on the origin just return
	if(glm::length(f) < 0.0001f)
	{
		return;
	}
	
	glm::mat<3, 3, float, glm::defaultp> flip{ sgn(f.x), 0, 0,
					  0, sgn(f.y), 0,
					  0, 0, sgn(f.z) };
	
	for (int l = 0; l < _model.m_meshes.size(); l++)
	{
		for (int j = 0; j < _model.m_meshes[l].positions.size(); j++)
		{
			_model.m_meshes[l].positions[j] = _model.m_meshes[l].positions[j] * flip;
		}
	}
}

void Normalizer::Remesh(ModelDescriptor& model)
{
	//The folder where we will save the subdivided mesh
	fs::path modifiedMeshesPath = fs::path("..\\ModifiedMeshes");

	auto newPath = modifiedMeshesPath;
	newPath /= model.m_path.filename();
	int error = system(("..\\Scripts\\meshlabserver.exe -s ..\\Scripts\\RM.mlx -i " + model.m_path.string() + " -o " + newPath.string()).c_str());
	if(error != 0)
	{
		std::cerr << "Failed to remesh" << "\n";
		return;
	}

	std::shared_ptr<Model> mdl = ModelLoader::LoadModel(newPath);
	if(mdl != nullptr)
	{
		model.m_model = mdl;
	}
	else
	{
		std::cerr << "Failed to remesh" << "\n";
	}

}
