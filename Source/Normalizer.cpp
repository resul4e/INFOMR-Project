#include "Normalizer.h"

#include <pmp/algorithms/SurfaceRemeshing.h>

#include <iostream>
#include "ModelLoader.h"

namespace fs = std::filesystem;

void Normalizer::Normalize(Model& _model)
{
	AlignModel(_model);
	CenterModel(_model);
	ScaleModel(_model);
	FlipModel(_model);
	_model.m_isUploaded = false;
}

void Normalizer::CenterModel(Model& _model)
{
	glm::vec3 mean{ 0,0,0 };
	for (int i = 0; i < 3; i++)
	{
		int positionAmounts = 0;
		for (int k = 0; k < _model.m_meshes.size(); k++)
		{
			positionAmounts += _model.m_meshes[k].positions.size();
			for (int j = 0; j < _model.m_meshes[k].positions.size(); j++)
			{
				mean[i] += _model.m_meshes[k].positions[j][i];
			}
		}
		mean[i] /= static_cast<float>(positionAmounts);
	}

	for (int k = 0; k < _model.m_meshes.size(); k++)
	{
		for (int j = 0; j < _model.m_meshes[k].positions.size(); j++)
		{
			_model.m_meshes[k].positions[j] -= mean;
		}
	}
}

void Normalizer::ScaleModel(Model& _model)
{
	//find the maximum and minimum value for each of the three axes.
	float largestDiff = 0;
	glm::vec3 max{ std::numeric_limits<float>::min() };
	glm::vec3 min{ std::numeric_limits<float>::max() };
	for (int i = 0; i < _model.m_meshes.size(); i++)
	{
		for (int j = 0; j < _model.m_meshes[i].positions.size(); j++)
		{
			for (int k = 0; k < 3; k++)
			{
				max[k] = std::max(max[k], _model.m_meshes[i].positions[j][k]);
				min[k] = std::min(min[k], _model.m_meshes[i].positions[j][k]);
			}
		}
	}

	//find the largest difference in one of the three axes.
	for (int k = 0; k < 3; k++)
	{
		float diff = max[k] - min[k];
		if (diff > largestDiff)
		{
			largestDiff = diff;
		}
	}

	//divide each position by the largest difference.
	for (int l = 0; l < _model.m_meshes.size(); l++)
	{
		for (int j = 0; j < _model.m_meshes[l].positions.size(); j++)
		{
			_model.m_meshes[l].positions[j] /= largestDiff;
		}
	}
}

void Normalizer::AlignModel(Model& _model)
{
	//The folder where we will save the subdivided mesh
	fs::path modifiedMeshesPath = fs::path("..\\ModifiedMeshes");
	
	auto newPath = modifiedMeshesPath;
	newPath /= _model.m_path.filename();
	system(("..\\Scripts\\mesh_filter.exe " + _model.m_path.string() + " -pcasnap " + newPath.string()).c_str());

	std::shared_ptr<Model> newModel = ModelLoader::LoadModel(newPath);
	std::swap(_model, *newModel);
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
}
