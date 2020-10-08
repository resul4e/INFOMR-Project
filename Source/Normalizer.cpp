#include "Normalizer.h"

#include "ModelUtil.h"

#include <pmp/algorithms/SurfaceRemeshing.h>

#include <iostream>
#include "ModelLoader.h"
#include "ModelSaver.h"
#include "ModelAnalytics.h"

#include "glm/gtx/component_wise.hpp"

namespace fs = std::filesystem;

void Normalizer::Normalize(Model& _model)
{
	AlignModel(_model);
	CenterModel(_model);
	FlipModel(_model);
	ScaleModel(_model);

	_model.UpdateFeatures();
	_model.m_isUploaded = false;
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
	////The folder where we will save the subdivided mesh
	//fs::path modifiedMeshesPath = fs::path("..\\ModifiedMeshes");
	//
	//auto newPath = modifiedMeshesPath;
	//newPath /= _model.m_path.filename();
	//system(("..\\Scripts\\mesh_filter.exe " + _model.m_path.string() + " -pcarot " + newPath.string()).c_str());

	//std::shared_ptr<Model> newModel = ModelLoader::LoadModel(newPath);
	//std::swap(_model, *newModel);
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

void Normalizer::Remesh(Model& model)
{
	std::vector<pmp::SurfaceMesh> pmpMeshes;
	model.ToPmpModel(pmpMeshes);

	for (pmp::SurfaceMesh& pmpMesh : pmpMeshes)
	{
		auto bb = pmpMesh.bounds().size();
		pmp::SurfaceRemeshing(pmpMesh).adaptive_remeshing(
			0.0010 * bb,  // min length
			0.010 * bb,  // max length
			0.0005 * bb
		); // approx. error
	}

	model.FromPmpModel(pmpMeshes);
	model.UpdateFeatures();
}
