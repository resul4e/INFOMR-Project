#pragma once

#include <glm/fwd.hpp>

class Model;

namespace analytics
{
	glm::vec3 ComputeExtents(const Model& model);

	float ComputeLongestAxis(const Model& model);

	float ComputeAbsCosineMajorEigenToXAxis(const Model& model);
}
