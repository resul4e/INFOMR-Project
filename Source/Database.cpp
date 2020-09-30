#include "Database.h"

#include <string>
#include <iostream>
#include <fstream>

#include "ModelLoader.h"
#include "Model.h"
#include "Normalizer.h"
#include "ModelSaver.h"
#include "ModelAnalytics.h"

namespace fs = std::filesystem;

/**
 * @brief Checks if a string ends with a certain ending.
 * @param fullString The string we want to check.
 * @param ending The ending we are going to check.
 * @return True if the ending matches, False otherwise.
*/
bool hasEnding(std::string const& fullString, std::string const& ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

void Database::AddModel(std::shared_ptr<Model> _model)
{
	m_modelDatabase.push_back(_model);
}

std::vector<std::shared_ptr<Model>> Database::GetModelDatabase()
{
	return m_modelDatabase;
}

void Database::LoadLabelledPSB(const fs::path& _labelledPSBDirectory)
{
	//Go through each directory and load the .off files
	for (const fs::directory_entry& p : fs::directory_iterator(_labelledPSBDirectory))
	{
		//Get the class name from the directory name.
		std::string cls = p.path().filename().string();
		for (const fs::directory_entry& m : fs::directory_iterator(p.path()))
		{
			if (m.path().extension() == ".off")
			{
				std::string fileName = m.path().filename().string();

				std::shared_ptr<Model> model = LoadModifiedModel(fileName);
				if(model == nullptr)
				{
					model = ModelLoader::LoadModel(m.path());
				}
				model->m_class = cls;
				model->m_name = cls + fileName;
				AddModel(model);
			}
		}
	}
}

void Database::LoadPSB(const fs::path& _PSBDirectory)
{
	fs::path modelDirectoryPath(_PSBDirectory / "db");
	fs::path testClassificationPath = _PSBDirectory / "classification" / "v1" / "coarse1" / "coarse1Test.cla";
	fs::path trainClassificationPath = _PSBDirectory / "classification" / "v1" / "coarse1" / "coarse1Train.cla";
	ReadPSBClassificationFile(modelDirectoryPath, testClassificationPath);
	ReadPSBClassificationFile(modelDirectoryPath, trainClassificationPath);
}

void Database::ReadPSBClassificationFile(fs::path _modelDirectoryPath, fs::path _filePath)
{
	std::string cls = "NO_CLASS_DETECTED!";
	
	int counter = 0;
	std::string line;
	std::ifstream testClassifications(_filePath);
	
	if (testClassifications.is_open())
	{
		while (std::getline(testClassifications, line))
		{
			//Check if we can ignore the line
			//We can do this when the line we are reading is empty, is some header information, or a baseclass without any models.
			counter++;
			if (line.empty() || counter <= 2 || hasEnding(line, "0 0"))
			{
				continue;
			}

			//Check if the line is a model ID or a shape class name.
			if (isdigit(line[0]))
			{
				//Extract the database this model belongs to.
				std::string db = "0";
				if (line.size() == 3)
				{
					db = line.substr(0, 1);
				}
				else if (line.size() == 4)
				{
					db = line.substr(0, 2);
				}

				fs::path modelPath = _modelDirectoryPath / db / ("m" + line) / ("m" + line + ".off");
				std::shared_ptr<Model> model = LoadModifiedModel(("m" + line + ".off"));
				if (model == nullptr)
				{
					model = ModelLoader::LoadModel(modelPath);
				}
				model->m_class = cls;
				model->m_name = cls + line;
				AddModel(model);
			}
			else
			{
				//Store the latest shape class because all subsequent model IDs will be part of this class.
				cls = line.substr(0, line.find(' '));
			}
		}
		testClassifications.close();
	}
}

void Database::ProcessAllModels()
{
	fs::path modifiedMeshesPath = fs::path("..\\ModifiedMeshes");
	fs::create_directory(modifiedMeshesPath);
	for(std::shared_ptr<Model>& model : m_modelDatabase)
	{
		SubdivideModel(model);
		CrunchModel(model);
	}
}

void Database::SaveAllModels()
{
	fs::path modifiedMeshesPath = fs::path("..\\ModifiedMeshes");
	fs::create_directory(modifiedMeshesPath);
	for (std::shared_ptr<Model>& model : m_modelDatabase)
	{
		ModelSaver::SavePly(*model, modifiedMeshesPath / model->m_path.filename().replace_extension(".ply"));
	}
}

void Database::NormalizeAllModels()
{
	analytics::DataRecorder barycenterRecorder;
	analytics::DataRecorder alignmentRecorder;
	analytics::DataRecorder scaleRecorder;

	for (std::shared_ptr<Model>& model : m_modelDatabase)
	{
		barycenterRecorder.preRecord(analytics::ComputeBarycenterDistance(*model));
		alignmentRecorder.preRecord(analytics::ComputeAbsCosineMajorEigenToXAxis(*model));
		scaleRecorder.preRecord(analytics::ComputeLongestAxis(*model));

		Normalizer::Normalize(*model);

		barycenterRecorder.postRecord(analytics::ComputeBarycenterDistance(*model));
		alignmentRecorder.postRecord(analytics::ComputeAbsCosineMajorEigenToXAxis(*model));
		scaleRecorder.postRecord(analytics::ComputeLongestAxis(*model));
	}

	barycenterRecorder.saveData("barycenters.csv");
	alignmentRecorder.saveData("alignment.csv");
	scaleRecorder.saveData("scale.csv");
}

void Database::SubdivideModel(std::shared_ptr<Model>& _model)
{
	//The folder where we will save the subdivided mesh
	fs::path modifiedMeshesPath = fs::path("..\\ModifiedMeshes");

	//check if we need to subdivide.
	bool subdivide = false;
	for (const Mesh& mesh : _model->m_meshes)
	{
		if (mesh.positions.size() < 1000 || mesh.faces.size() < 1000)
		{
			subdivide = true;
			std::cerr << "Not enough verts/faces in model with name: " << _model->m_name << std::endl;
		}
	}

	//If we do need to subidivde: call the script and load the model, then recall this method to see if the
	//new model has enough verts/faces. If not do this again. Once this recursive call is returned swap the
	//new model with the old one so that we immediately have access to the higher fidelity model.
	if (subdivide)
	{
		auto newPath = modifiedMeshesPath;
		newPath /= _model->m_path.filename();
		system(("..\\Scripts\\mesh_filter.exe " + _model->m_path.string() + " -subdiv " + newPath.string()).c_str());

		std::shared_ptr<Model> newModel = ModelLoader::LoadModel(newPath);
		SubdivideModel(newModel);
		_model.swap(newModel);
	}
}

void Database::CrunchModel(std::shared_ptr<Model>& _model)
{
	//The folder where we will save the crunched mesh
	fs::path modifiedMeshesPath = fs::path("..\\ModifiedMeshes");
	
	for (const Mesh& mesh : _model->m_meshes)
	{
		if (mesh.positions.size() > 20000 || mesh.faces.size() > 20000)
		{
			auto newPath = modifiedMeshesPath;
			newPath /= _model->m_path.filename();
			system(("..\\Scripts\\mesh_crunch.exe " + _model->m_path.string() + " " + newPath.string()).c_str());

			std::shared_ptr<Model> newModel = ModelLoader::LoadModel(newPath);
			_model.swap(newModel);
		}
	}
}

void Database::SortDatabase(SortingOptions _option)
{
	switch (_option)
	{
	case SortingOptions::VERTEX_COUNT:
		std::sort(m_modelDatabase.begin(), m_modelDatabase.end(), [](const std::shared_ptr<Model>& _left, const std::shared_ptr<Model>& _right)
	{
				return _left->m_vertexCount < _right->m_vertexCount;
	});
		break;
	case SortingOptions::FACE_COUNT:
		std::sort(m_modelDatabase.begin(), m_modelDatabase.end(), [](const std::shared_ptr<Model>& _left, const std::shared_ptr<Model>& _right)
			{
				return _left->m_faceCount < _right->m_faceCount;
			});
		break;
	case SortingOptions::BOUNDS:
		std::sort(m_modelDatabase.begin(), m_modelDatabase.end(), [](const std::shared_ptr<Model>& _left, const std::shared_ptr<Model>& _right)
			{
				return glm::length(_left->m_bounds.max - _left->m_bounds.min) < glm::length(_right->m_bounds.max - _right->m_bounds.min);
			});
		break;
	default:
		break;
	}
}

std::shared_ptr<Model> Database::LoadModifiedModel(std::filesystem::path _modelFileName)
{
	fs::path modifiedMeshesPath = fs::path("..\\ModifiedMeshes");

	fs::path offPath = _modelFileName;
	fs::path plyPath = _modelFileName.replace_extension(".ply");
	if (fs::exists(modifiedMeshesPath / plyPath))
	{
		return ModelLoader::LoadModel(modifiedMeshesPath / plyPath);
	}

	if(fs::exists(modifiedMeshesPath / offPath))
	{
		return ModelLoader::LoadModel(modifiedMeshesPath / offPath);
	}

	return nullptr;
}
