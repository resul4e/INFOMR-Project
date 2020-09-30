#pragma once

#include <glm/fwd.hpp>

class Model;

namespace analytics
{
	float ComputeBarycenterDistance(const Model& model);

	glm::vec3 ComputeExtents(const Model& model);

	float ComputeLongestAxis(const Model& model);

	float ComputeAbsCosineMajorEigenToXAxis(const Model& model);
}
