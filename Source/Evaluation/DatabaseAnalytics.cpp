#include "DatabaseAnalytics.h"

#include "Database.h"
#include "ModelDescriptor.h"

#include <fstream>

namespace analytics
{
	void ComputeFeatureDistribution(std::string fileName, DescriptorName descName, Database& database)
	{
		std::ofstream outFile;
		outFile.open(fileName);

		outFile << database.getFeatureAverages()[descName] << std::endl;
		outFile << database.getFeatureStddevs()[descName] << std::endl;
		for (ModelDescriptor& md : database.GetModelDatabase())
		{
			float value = md.m_3DFeatures[descName];
			outFile << value << std::endl;
		}
		outFile.close();
	}
}
