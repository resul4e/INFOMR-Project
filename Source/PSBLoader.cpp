#include "PSBLoader.h"

#include "Database.h"
#include "ModelDescriptor.h"

#include <filesystem>
#include <fstream>
#include "ModelLoader.h"

namespace fs = std::filesystem;

namespace
{
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

	void ReadPSBClassificationFile(fs::path _modelDirectoryPath, fs::path _filePath, Database& _database)
	{
		const fs::path descriptorDatabasePath("DescriptorDatabase");
		
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

					int vertexCount, faceCount;
					ModelLoader::LoadDescriptorData(descriptorDatabasePath / modelPath.filename().replace_extension("csv"), vertexCount, faceCount);
					descriptor.m_faceCount = vertexCount;
					descriptor.m_vertexCount = faceCount;

					_database.AddModel(descriptor);
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
}

namespace io
{
	void LoadLabelledPSB(const fs::path& _labelledPSBDirectory, Database& _database)
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

					_database.AddModel(descriptor);
				}
			}
		}
	}

	void LoadPSB(const fs::path& _PSBDirectory, Database& _database)
	{
		fs::path modelDirectoryPath(_PSBDirectory / "db");
		fs::path testClassificationPath = _PSBDirectory / "classification" / "v1" / "coarse1" / "coarse1Test.cla";
		fs::path trainClassificationPath = _PSBDirectory / "classification" / "v1" / "coarse1" / "coarse1Train.cla";
		ReadPSBClassificationFile(modelDirectoryPath, testClassificationPath, _database);
		ReadPSBClassificationFile(modelDirectoryPath, trainClassificationPath, _database);

		_database.LoadFeatureDatabase();
	}
}
