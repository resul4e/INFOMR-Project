#pragma once

#include <glm/fwd.hpp>

#include <string>
#include <vector>

class Model;

namespace analytics
{
	class DataRecorder
	{
	public:
		DataRecorder();

		void preRecord(float data);
		void postRecord(float data);

		void saveData(std::string fileName);

	private:
		std::vector<float> m_preData;
		std::vector<float> m_postData;
	};

	float ComputeBarycenterDistance(const Model& model);

	glm::vec3 ComputeExtents(const Model& model);

	float ComputeLongestAxis(const Model& model);

	float ComputeAbsCosineMajorEigenToXAxis(const Model& model);
}
