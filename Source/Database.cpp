#include "Database.h"

#include <string>
#include <iostream>
#include <fstream>

#include "ModelLoader.h"
#include "Model.h"
#include "Normalizer.h"
#include "ModelSaver.h"
#include "ModelAnalytics.h"

#include "pmp/algorithms/SurfaceSubdivision.h"
#include "pmp/algorithms/SurfaceNormals.h"

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

void Database::AddModel(ModelDescriptor _model)
{
	m_modelDatabase.push_back(_model);
}

std::vector<ModelDescriptor> Database::GetModelDatabase()
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
				std::string modelPath = m.path().string();
				std::string modelName = m.path().filename().replace_extension("").string();

				ModelDescriptor descriptor;
				descriptor.m_class = cls;
				descriptor.m_name = cls + modelName;
				descriptor.m_path = modelPath;

				AddModel(descriptor);
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

				ModelDescriptor descriptor;
				descriptor.m_class = cls;
				descriptor.m_name = cls + line;
				descriptor.m_path = modelPath;

				AddModel(descriptor);
				std::cout << "Added model: " << descriptor.m_name << std::endl;
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
	const fs::path featureDatabasePath = fs::path("..\\FeatureDatabase");
	const fs::path savedMeshesPath = fs::path("..\\SavedMeshes");
	fs::create_directory(savedMeshesPath);

	Features3D::globalBoundsA3.s = 0;
	Features3D::globalBoundsA3.t = 3.14159f;
	Features3D::globalBoundsD1.s = 0;
	Features3D::globalBoundsD1.t = 1;
	Features3D::globalBoundsD2.s = 0;
	Features3D::globalBoundsD2.t = std::sqrt(2) + 0.001f;
	Features3D::globalBoundsD3.s = 0;
	Features3D::globalBoundsD3.t = 0.86660254038;
	Features3D::globalBoundsD4.s = 0;
	Features3D::globalBoundsD4.t = 1.0f/3.0f;

	SortDatabase(Database::SortingOptions::BOUNDS);
	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
	{
		if (fs::exists(featureDatabasePath / modelDescriptor.m_path.filename().replace_extension(".csv")))
		{
			continue;
		}
		
		if(!fs::exists(savedMeshesPath / modelDescriptor.m_path.filename().replace_extension(".ply")))
		{
			modelDescriptor.m_model = ModelLoader::LoadModel(std::filesystem::path(modelDescriptor.m_path));
			SubdivideModel(modelDescriptor);
			CrunchModel(modelDescriptor);
			Normalizer::Remesh(modelDescriptor);
			Normalizer::Normalize(modelDescriptor);
		}
		else
		{
			modelDescriptor.m_model = LoadSavedModel(modelDescriptor.m_path);
			if(modelDescriptor.m_model == nullptr)
			{
				continue;
			}
			Normalizer::Normalize(modelDescriptor);
		}

		modelDescriptor.UpdateBounds();
		modelDescriptor.UpdateFeatures();
		ModelSaver::SavePly(modelDescriptor, savedMeshesPath / modelDescriptor.m_path.filename().replace_extension(".ply"));
		modelDescriptor.m_model = nullptr;
	}

	CompoundHistogramPerClass();
}

void Database::RemeshAllModels()
{
	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
	{
		Normalizer::Remesh(modelDescriptor);
	}
}

void Database::SaveAllModels()
{
	const fs::path savedMeshesPath = fs::path("..\\SavedMeshes");
	fs::create_directory(savedMeshesPath);
	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
	{
		ModelSaver::SavePly(modelDescriptor, savedMeshesPath / modelDescriptor.m_path.filename().replace_extension(".ply"));
	}
}

void Database::NormalizeAllModels()
{
	analytics::DataRecorder barycenterRecorder;
	analytics::DataRecorder alignmentRecorder;
	analytics::DataRecorder scaleRecorder;

	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
	{
		barycenterRecorder.preRecord(analytics::ComputeBarycenterDistance(*modelDescriptor.m_model));
		alignmentRecorder.preRecord(analytics::ComputeAbsCosineMajorEigenToXAxis(*modelDescriptor.m_model));
		scaleRecorder.preRecord(analytics::ComputeLongestAABBAxis(*modelDescriptor.m_model));

		Normalizer::Normalize(modelDescriptor);

		barycenterRecorder.postRecord(analytics::ComputeBarycenterDistance(*modelDescriptor.m_model));
		alignmentRecorder.postRecord(analytics::ComputeAbsCosineMajorEigenToXAxis(*modelDescriptor.m_model));
		scaleRecorder.postRecord(analytics::ComputeLongestAABBAxis(*modelDescriptor.m_model));
	}

	barycenterRecorder.saveData("barycenters.csv");
	alignmentRecorder.saveData("alignment.csv");
	scaleRecorder.saveData("scale.csv");
}

void Database::SubdivideModel(ModelDescriptor& _modelDescriptor)
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

	//If we do need to subidivde: call the script and load the model, then recall this method to see if the
	//new model has enough verts/faces. If not do this again. Once this recursive call is returned swap the
	//new model with the old one so that we immediately have access to the higher fidelity model.
	if (subdivide)
	{
		auto newPath = modifiedMeshesPath;

		newPath /= _modelDescriptor.m_path.filename();
		newPath.replace_extension(".ply");
		auto command = ("meshlabserver.exe -s ..\\Scripts\\SubdivOnce.mlx -i " + _modelDescriptor.m_path.string() + " -o " + newPath.string());
		int error = system(command.c_str());
		if(error != 0)
		{
			std::cerr << "Subdivision failed', using backup subdivision" << "\n";
			system(("..\\Scripts\\mesh_filter.exe " + _modelDescriptor.m_path.string() + " -subdiv " + newPath.string()).c_str());
		}

		_modelDescriptor.m_model = ModelLoader::LoadModel(newPath);
		_modelDescriptor.UpdateFeatures();
	}
}

void Database::CrunchModel(ModelDescriptor& _modelDescriptor)
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
			_modelDescriptor.UpdateFeatures();
		}
	}
}

void Database::SortDatabase(SortingOptions _option)
{
	switch (_option)
	{
	case SortingOptions::VERTEX_COUNT:
		std::sort(m_modelDatabase.begin(), m_modelDatabase.end(), [](const ModelDescriptor& _left, const ModelDescriptor& _right)
	{
				return _left.m_vertexCount < _right.m_vertexCount;
	});
		break;
	case SortingOptions::FACE_COUNT:
		std::sort(m_modelDatabase.begin(), m_modelDatabase.end(), [](const ModelDescriptor& _left, const ModelDescriptor& _right)
			{
				return _left.m_faceCount < _right.m_faceCount;
			});
		break;
	case SortingOptions::BOUNDS:
		std::sort(m_modelDatabase.begin(), m_modelDatabase.end(), [](const ModelDescriptor& _left, const ModelDescriptor& _right)
			{
				return glm::length(_left.m_bounds.max - _left.m_bounds.min) < glm::length(_right.m_bounds.max - _right.m_bounds.min);
			});
		break;
	default:
		break;
	}
}

void Database::CompoundHistogramPerClass()
{
	const fs::path featureDatabasePath = fs::path("..\\FeatureDatabase");
	fs::create_directory(featureDatabasePath / "Combined");
	std::map<std::string, std::vector<ModelDescriptor>> classMap;
	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
	{
		if(!fs::exists(featureDatabasePath / modelDescriptor.m_path.filename().replace_extension(".csv")))
		{
			continue;
		}
		
		modelDescriptor.m_3DFeatures = ModelLoader::LoadFeatures(featureDatabasePath / modelDescriptor.m_path.filename().replace_extension(".csv"));

		if(classMap.find(modelDescriptor.m_class) == classMap.end())
		{
			classMap.insert({ modelDescriptor.m_class, std::vector<ModelDescriptor>() });
		}
		classMap[modelDescriptor.m_class].push_back(modelDescriptor);
	}

	for(auto classDescPair : classMap)
	{
		std::ofstream outFile(featureDatabasePath / "Combined" / ("a3" + classDescPair.first + ".csv"));

		for(const ModelDescriptor& desc : classDescPair.second)
		{
			for (int i = 0; i < HISTOGRAM_BIN_SIZE; i++)
			{
				outFile << desc.m_3DFeatures.a3.binCount[i] << ", ";
			}
			outFile << "\n";
		}
		outFile << Features3D::globalBoundsA3.s << ", " << Features3D::globalBoundsA3.t;
		
		outFile.close();
	}

	for (auto classDescPair : classMap)
	{
		std::ofstream outFile(featureDatabasePath / "Combined" / ("d1" + classDescPair.first + ".csv"));

		for (const ModelDescriptor& desc : classDescPair.second)
		{
			for (int i = 0; i < HISTOGRAM_BIN_SIZE; i++)
			{
				outFile << desc.m_3DFeatures.d1.binCount[i] << ", ";
			}
			outFile << "\n";
		}
		outFile << Features3D::globalBoundsD1.s << ", " << Features3D::globalBoundsD1.t;

		outFile.close();
	}

	for (auto classDescPair : classMap)
	{
		std::ofstream outFile(featureDatabasePath / "Combined" / ("d2" + classDescPair.first + ".csv"));

		for (const ModelDescriptor& desc : classDescPair.second)
		{
			for (int i = 0; i < HISTOGRAM_BIN_SIZE; i++)
			{
				outFile << desc.m_3DFeatures.d2.binCount[i] << ", ";
			}
			outFile << "\n";
		}
		outFile << Features3D::globalBoundsD2.s << ", " << Features3D::globalBoundsD2.t;

		outFile.close();
	}

	for (auto classDescPair : classMap)
	{
		std::ofstream outFile(featureDatabasePath / "Combined" / ("d3" + classDescPair.first + ".csv"));

		for (const ModelDescriptor& desc : classDescPair.second)
		{
			for (int i = 0; i < HISTOGRAM_BIN_SIZE; i++)
			{
				outFile << desc.m_3DFeatures.d3.binCount[i] << ", ";
			}
			outFile << "\n";
		}
		outFile << Features3D::globalBoundsD3.s << ", " << Features3D::globalBoundsD3.t;

		outFile.close();
	}

	for (auto classDescPair : classMap)
	{
		std::ofstream outFile(featureDatabasePath / "Combined" / ("d4" + classDescPair.first + ".csv"));

		for (const ModelDescriptor& desc : classDescPair.second)
		{
			for (int i = 0; i < HISTOGRAM_BIN_SIZE; i++)
			{
				outFile << desc.m_3DFeatures.d4.binCount[i] << ", ";
			}
			outFile << "\n";
		}
		outFile << Features3D::globalBoundsD4.s << ", " << Features3D::globalBoundsD4.t;

		outFile.close();
	}
}

std::shared_ptr<Model> Database::LoadSavedModel(std::filesystem::path _modelFileName)
{
	//TODO(Resul): Remove this return
	//For now just always load the default model

	fs::path modifiedMeshesPath = fs::path("..\\SavedMeshes");

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
