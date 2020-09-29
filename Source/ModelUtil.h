#pragma once

#include <glm/fwd.hpp>

class Model;

namespace util
{
	glm::vec3 ComputeBarycenter(const Model& model);

	void ComputeEigenVectors(const Model& model, glm::vec3& eVec1, glm::vec3& eVec2, glm::vec3& eVec3, glm::vec3& eValues);

	void RotateMajorEigenVectorToXAxis(Model& model);
}
