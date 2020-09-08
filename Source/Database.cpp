#include "Database.h"

#include <string>
#include <iostream>
#include <fstream>

#include "ModelLoader.h"
#include "Model.h"

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

void Database::LoadLabelledPSB(fs::path _labelledPSBDirectory)
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
				std::shared_ptr<Model> model = ModelLoader::LoadModel(m.path());
				model->m_class = cls;
				model->m_name = cls + m.path().filename().string();
				AddModel(model);
			}
		}
	}
}

void Database::LoadPSB(std::filesystem::path _PSBDirectory)
{
	int counter = 0;
	std::string line;
	std::string cls = "NO_CLASS_DETECTED!";
	fs::path modelDirectoryPath(_PSBDirectory / "db");
	std::ifstream testClassifications(_PSBDirectory / "classification" / "v1" / "coarse1" / "coarse1Test.cla");
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

				fs::path modelPath = modelDirectoryPath / db / ("m" + line) / ("m" + line + ".off");
				std::shared_ptr<Model> model = ModelLoader::LoadModel(modelPath);
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
	
	for(std::shared_ptr<Model> model : m_modelDatabase)
	{
		bool subdivide = false;
		for(const Mesh& mesh : model->m_meshes)
		{
			if(mesh.positions.size() < 100 || mesh.faces.size() < 100)
			{
				subdivide = true;
				std::cerr << "Not enough verts/faces in model with name: " << model->m_name << std::endl;
			}
		}
		if(subdivide)
		{
			auto newPath = model->m_path;
			newPath = newPath.replace_extension("");
			newPath = newPath.replace_filename(newPath.filename().string() + "1");
			newPath = newPath.replace_extension(".off");
			system(("java -jar ../Scripts/catmullclark.jar "+ model->m_path.string() + " " + newPath.string()).c_str());
		}
	}
}
