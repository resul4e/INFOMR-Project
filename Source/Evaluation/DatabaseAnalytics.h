#pragma once

#include <string>

class Database;
enum DescriptorName;

namespace analytics
{
	void ComputeFeatureDistribution(std::string fileName, DescriptorName descName, Database& database);
}
