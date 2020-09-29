#include "ModelAnalytics.h"

#include "Model.h"
#include "ModelUtil.h"

#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>

namespace analytics
{
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
		
		return 0;
	}
}
