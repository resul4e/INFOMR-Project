#pragma once

#include <glm/fwd.hpp>

class Model;

namespace util
{
	glm::vec3 ComputeBarycenter(const Model& model);

	void ComputeAABB(const Model& model, glm::vec3& min, glm::vec3& max);

	void ComputeEigenVectors(const Model& model, glm::vec3& eVec1, glm::vec3& eVec2, glm::vec3& eVec3, glm::vec3& eValues);

	void RotateMajorEigenVectorToXAxis(Model& model);

	void GetSortedEigenValues(const Model& model, glm::vec3& eigenValues);
	
	void GetSortedEigenVectors(const Model& model, glm::vec3& majorEigenVector, glm::vec3& medianEigenVector, glm::vec3& minorEigenVector);
	void GetSortedEigenVectors(glm::vec3* eigenVectors, glm::vec3& majorEigenVector, glm::vec3& medianEigenVector, glm::vec3& minorEigenVector, const glm::vec3& eValues);
}
