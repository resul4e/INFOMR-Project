#include "QueryManager.h"
#include <iostream>
#include <fstream>
#include <cctype>

#include "ModelLoader.h"
#include "Model.h"

namespace fs = std::filesystem;

QueryManager::QueryManager()
{
	m_database = std::make_shared<Database>();
}

QueryManager::~QueryManager()
{
}

void QueryManager::LoadLabelledPSB(fs::path _labelledPSBDirectory)
{
	for (const fs::directory_entry& p : fs::directory_iterator(_labelledPSBDirectory))
	{
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

bool hasEnding(std::string const& fullString, std::string const& ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
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
			counter++;
			if(line == "" || counter <= 2)
			{
				continue;
			}
			if(hasEnding(line, "0 0"))
			{
				continue;
			}

			if(isdigit(line[0]))
			{
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
				cls = line.substr(0, line.find(' '));
			}

			std::cout << line << "\n";
		}
		testClassifications.close();
	}
}

std::shared_ptr<Database> QueryManager::GetDatabase()
{
	return m_database;
}
