#include "ModelProcessing.h"

#include "ModelDescriptor.h"
#include "Model.h"
#include "ModelUtil.h"
#include "ModelLoader.h"

#include <glm/gtx/component_wise.hpp>

#include <iostream>

namespace fs = std::filesystem;

namespace
{
	void CenterModel(Model& _model)
	{
		glm::vec3 mean = util::ComputeBarycenter(_model);

		// Subtract mean from each vertex position
		for (Mesh& mesh : _model.m_meshes)
			for (glm::vec3& p : mesh.positions)
				p -= mean;
	}

	void ScaleModel(Model& _model)
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

	void AlignModel(Model& _model)
	{
		util::RotateMajorEigenVectorToXAxis(_model);
	}

	template <typename T> int sgn(T val) {
		return (T(0) < val) - (val < T(0));
	}

	void FlipModel(Model& _model)
	{
		glm::vec3 f{ 0,0,0 };
		for (int l = 0; l < _model.m_meshes.size(); l++)
		{
			for (int j = 0; j < _model.m_meshes[l].faces.size(); j++)
			{
				glm::vec3 center{ 0,0,0 };
				for (int k = 0; k < 3; k++)
				{
					center += _model.m_meshes[l].positions[_model.m_meshes[l].faces[j].indices[k]];
				}
				center /= 3;

				for (int k = 0; k < 3; k++)
				{
					f[k] += sgn(center[k]) * (center[k] * center[k]);
				}
			}
		}

		//If we are going to put all vertices on the origin just return
		if (glm::length(f) < 0.0001f)
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
}

namespace proc
{
	void Normalize(ModelDescriptor& _modelDescriptor)
	{
		AlignModel(*_modelDescriptor.m_model);
		CenterModel(*_modelDescriptor.m_model);
		FlipModel(*_modelDescriptor.m_model);
		ScaleModel(*_modelDescriptor.m_model);

		_modelDescriptor.UpdateBounds();
		_modelDescriptor.UpdateFeatures();
		_modelDescriptor.m_model->markForReupload();
	}


	void Remesh(ModelDescriptor& _modelDescriptor)
	{
		//The folder where we will save the subdivided mesh
		fs::path modifiedMeshesPath = fs::path("..\\ModifiedMeshes");

		auto newPath = modifiedMeshesPath;
		newPath /= _modelDescriptor.m_path.filename();
		int error = system(("..\\Scripts\\meshlabserver.exe -s ..\\Scripts\\RM.mlx -i " + _modelDescriptor.m_path.string() + " -o " + newPath.string()).c_str());
		if (error != 0)
		{
			std::cerr << "Failed to remesh" << "\n";
			return;
		}

		std::shared_ptr<Model> mdl = ModelLoader::LoadModel(newPath);
		if (mdl != nullptr)
		{
			_modelDescriptor.m_model = mdl;
		}
		else
		{
			std::cerr << "Failed to remesh" << "\n";
		}
	}

	void SubdivideModel(ModelDescriptor& _modelDescriptor)
	{
		//The folder where we will save the subdivided mesh
		fs::path modifiedMeshesPath = fs::path("..\\ModifiedMeshes");

		//check if we need to subdivide.
		bool subdivide = false;
		for (const Mesh& mesh : _modelDescriptor.m_model->m_meshes)
		{
			if (mesh.positions.size() < 1000 || mesh.faces.size() < 1000)
			{
				subdivide = true;
				std::cerr << "Not enough verts/faces in model with name: " << _modelDescriptor.m_name << std::endl;
			}
		}

		//If we do need to subdivide: call the script and load the model, then recall this method to see if the
		//new model has enough verts/faces. If not do this again. Once this recursive call is returned swap the
		//new model with the old one so that we immediately have access to the higher fidelity model.
		if (subdivide)
		{
			auto newPath = modifiedMeshesPath;

			newPath /= _modelDescriptor.m_path.filename();
			newPath.replace_extension(".ply");
			
			auto command = ("..\\Scripts\\meshlabserver.exe -s ..\\Scripts\\SubdivOnce.mlx -i " + _modelDescriptor.m_path.string() + " -o " + newPath.string());
			int error = system(command.c_str());
			if (error != 0)
			{
				std::cerr << "Subdivision failed', using backup subdivision" << "\n";
				system(("..\\Scripts\\mesh_filter.exe " + _modelDescriptor.m_path.string() + " -subdiv " + newPath.string()).c_str());
			}

			_modelDescriptor.m_model = ModelLoader::LoadModel(newPath);
		}
	}

	void CrunchModel(ModelDescriptor& _modelDescriptor)
	{
		//The folder where we will save the crunched mesh
		fs::path modifiedMeshesPath = fs::path("..\\ModifiedMeshes");

		for (const Mesh& mesh : _modelDescriptor.m_model->m_meshes)
		{
			if (mesh.positions.size() > 40000 || mesh.faces.size() > 40000)
			{
				auto newPath = modifiedMeshesPath;
				newPath /= _modelDescriptor.m_path.filename();
				system(("..\\Scripts\\mesh_crunch.exe " + _modelDescriptor.m_path.string() + " " + newPath.string()).c_str());

				_modelDescriptor.m_model = ModelLoader::LoadModel(newPath);
			}
		}
	}
}
