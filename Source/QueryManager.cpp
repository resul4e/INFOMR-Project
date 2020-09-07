#include "QueryManager.h"
#include <iostream>
#include <fstream>
#include <cctype>

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

QueryManager::QueryManager()
{
	m_database = std::make_shared<Database>();
}

QueryManager::~QueryManager()
{
}

void QueryManager::LoadLabelledPSB(fs::path _labelledPSBDirectory)
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
				m_database->AddModel(model);
			}
		}
	}
}

void QueryManager::LoadPSB(std::filesystem::path _PSBDirectory)
{
	int counter = 0;
	std::string line;
	std::string cls = "NO_CLASS_DETECTED!";
	fs::path modelPath(_PSBDirectory / "db");
	std::ifstream testClassifications(_PSBDirectory / "classification" / "v1" / "coarse1" / "coarse1Test.cla");
	if (testClassifications.is_open())
	{
		while (std::getline(testClassifications, line))
		{
			//Check if we can ignore the line
			//We can do this when the line we are reading is empty, is some header information, or a baseclass without any models.
			counter++;
			if(line.empty() || counter <= 2 || hasEnding(line, "0 0"))
			{
				continue;
			}

			//Check if the line is a model ID or a shape class name.
			if(isdigit(line[0]))
			{
				//Extract the database this model belongs to.
				std::string db = "0";
				if(line.size() == 3)
				{
					db = line.substr(0, 1);
				}
				else if(line.size() == 4)
				{
					db = line.substr(0, 2);
				}
				
				std::shared_ptr<Model> model = ModelLoader::LoadModel(modelPath / db / ("m" + line) / ("m" + line + ".off"));
				model->m_class = cls;
				m_database->AddModel(model);
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

std::shared_ptr<Database> QueryManager::GetDatabase()
{
	return m_database;
}
