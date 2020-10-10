#include "ModelUtil.h"

#include "Model.h"
#include <Eigen/Core>
#include <Eigen/Eigenvalues>

#include <glm/glm.hpp>

namespace util
{
	glm::vec3 ComputeBarycenter(const Model& model)
	{
		glm::vec3 barycenter(0, 0, 0);
		unsigned int vertexCount = 0;

		for (const Mesh& mesh : model.m_meshes)
		{
			for (const glm::vec3& p : mesh.positions)
				barycenter += p;
			vertexCount += mesh.positions.size();
		}
		barycenter /= vertexCount;

		return barycenter;
	}
	
	void ComputeAABB(const Model& model, glm::vec3& min, glm::vec3& max)
	{
		min = glm::vec3{ std::numeric_limits<float>::max() };
		max = glm::vec3{ -std::numeric_limits<float>::max() };

		for (const Mesh& mesh : model.m_meshes)
		{
			for (const glm::vec3& p : mesh.positions)
			{
				for (int d = 0; d < 3; d++)
				{
					min[d] = p[d] < min[d] ? p[d] : min[d];
					max[d] = p[d] > max[d] ? p[d] : max[d];
				}
			}
		}
	}

	void ComputeEigenVectors(const Model& model, glm::vec3& _majorEigenVector, glm::vec3& _medianEigenVector, glm::vec3& _minorEigenVector, glm::vec3& _eigenValues)
	{
		glm::vec3 barycenter = ComputeBarycenter(model);

		Eigen::Matrix3d covariance;
		covariance.setZero();
		unsigned int vertexCount = 0;
		for (const Mesh& mesh : model.m_meshes)
		{
			for (const glm::vec3& p : mesh.positions)
			{
				glm::vec3 dev = p - barycenter;
				Eigen::Vector3d eigenDev = Eigen::Vector3d{ dev.x, dev.y, dev.z };
				covariance += eigenDev * eigenDev.transpose();
			}
			vertexCount += mesh.positions.size();
		}
		covariance /= (double)vertexCount;

		// Compute eigenvectors for the covariance matrix
		Eigen::EigenSolver<Eigen::Matrix3d> solver(covariance);
		Eigen::Matrix3d eVectors = solver.eigenvectors().real();
		Eigen::Vector3d eValues = solver.eigenvalues().real();

		// Extract to glm vectors
		std::vector<glm::vec3> eigenVectors(3);
		eigenVectors[0] = glm::vec3(eVectors.col(0).x(), eVectors.col(0).y(), eVectors.col(0).z());
		eigenVectors[1] = glm::vec3(eVectors.col(1).x(), eVectors.col(1).y(), eVectors.col(1).z());
		eigenVectors[2] = glm::vec3(eVectors.col(2).x(), eVectors.col(2).y(), eVectors.col(2).z());
		glm::vec3 eigenValues = glm::vec3(eValues.x(), eValues.y(), eValues.z());

		// Sort eigenvectors and eigenvalues
		std::vector<int> sortedIndices = { 0, 1, 2 };
		std::vector<float> sortedEigenValues = { eigenValues.x, eigenValues.y, eigenValues.z };
		std::sort(std::begin(sortedIndices), std::end(sortedIndices), [&](int i1, int i2) { return sortedEigenValues[i1] > sortedEigenValues[i2]; });

		_majorEigenVector = eigenVectors[sortedIndices[0]];
		_medianEigenVector = eigenVectors[sortedIndices[1]];
		_minorEigenVector = glm::cross(_majorEigenVector, _medianEigenVector);
		_eigenValues = glm::vec3(sortedEigenValues[0], sortedEigenValues[1], sortedEigenValues[2]);
	}

	void RotateMajorEigenVectorToXAxis(Model& model)
	{
		glm::vec3 eigenVectors[3];
		glm::vec3 eigenValues;
		util::ComputeEigenVectors(model, eigenVectors[0], eigenVectors[1], eigenVectors[2], eigenValues);

		std::vector<int> sortedIndices = { 0, 1, 2 };
		std::vector<float> sortedEigenValues = { eigenValues.x, eigenValues.y, eigenValues.z };
		std::sort(std::begin(sortedIndices), std::end(sortedIndices), [&](int i1, int i2) { return sortedEigenValues[i1] > sortedEigenValues[i2]; });

		glm::vec3 majorEigenVector;
		glm::vec3 medianEigenVector;
		glm::vec3 minorEigenVector;

		ComputeEigenVectors(model, majorEigenVector, medianEigenVector, minorEigenVector, eigenValues);
		
		glm::mat3x3 rotMat{ majorEigenVector.x, medianEigenVector.x, minorEigenVector.x,
							majorEigenVector.y, medianEigenVector.y, minorEigenVector.y,
							majorEigenVector.z, medianEigenVector.z, minorEigenVector.z };

		for (Mesh& mesh : model.m_meshes)
			for (glm::vec3& p : mesh.positions)
				p = rotMat * p;
	}
}
