#include "ModelAnalytics.h"

#include "Model.h"
#include "ModelUtil.h"

#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>

#include <fstream>

namespace analytics
{
	DataRecorder::DataRecorder()
	{

	}

	void DataRecorder::preRecord(float data)
	{
		m_preData.push_back(data);
	}

	void DataRecorder::postRecord(float data)
	{
		m_postData.push_back(data);
	}
	
	void DataRecorder::saveData(std::string fileName)
	{
		std::ofstream outFile;
		outFile.open(fileName);
		for (int i = 0; i < m_preData.size(); i++)
		{
			outFile << m_preData[i] << "," << m_postData[i] << "\n";
		}
		outFile.close();
	}
	float ComputeBarycenterDistance(const Model& model)
	{
		return glm::length(util::ComputeBarycenter(model));
	}

	glm::vec3 ComputeExtents(const Model& model)
	{
		glm::vec3 extent(0, 0, 0);

		for (const Mesh& mesh : model.m_meshes)
		{
			for (const glm::vec3& p : mesh.positions)
			{
				for (int d = 0; d < 3; d++)
					extent[d] = std::max(extent[d], p[d]);
			}
		}

		return extent;
	}

	float ComputeLongestAxis(const Model& model)
	{
		return glm::compMax(ComputeExtents(model));
	}

	float ComputeAbsCosineMajorEigenToXAxis(const Model& model)
	{
		glm::vec3 eigenVectors[3];
		glm::vec3 eigenValues;
		util::ComputeEigenVectors(model, eigenVectors[0], eigenVectors[1], eigenVectors[2], eigenValues);

		std::vector<int> sortedIndices = { 0, 1, 2 };
		std::vector<float> sortedEigenValues = { eigenValues.x, eigenValues.y, eigenValues.z };
		std::sort(std::begin(sortedIndices), std::end(sortedIndices), [&](int i1, int i2) { return sortedEigenValues[i1] > sortedEigenValues[i2]; });
		
		glm::vec3 majorEigenVector = eigenVectors[sortedIndices[0]];

		float absCos = abs(glm::dot(majorEigenVector, glm::vec3(1, 0, 0)));

		return absCos;
	}
}
