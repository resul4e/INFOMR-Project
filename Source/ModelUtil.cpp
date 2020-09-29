#include "ModelUtil.h"

#include "Model.h"

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

	void ComputeEigenVectors(const Model& model, glm::vec3& eVec1, glm::vec3& eVec2, glm::vec3& eVec3, glm::vec3& eValues)
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
		Eigen::Matrix3d eigenVectors = solver.eigenvectors().real();
		Eigen::Vector3d eigenValues = solver.eigenvalues().real();

		eVec1 = glm::vec3(eigenVectors.col(0).x(), eigenVectors.col(0).y(), eigenVectors.col(0).z());
		eVec2 = glm::vec3(eigenVectors.col(1).x(), eigenVectors.col(1).y(), eigenVectors.col(1).z());
		eVec3 = glm::vec3(eigenVectors.col(2).x(), eigenVectors.col(2).y(), eigenVectors.col(2).z());
		eValues = glm::vec3(eigenValues.x(), eigenValues.y(), eigenValues.z());
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

		GetSortedEigenVectors(eigenVectors, majorEigenVector, medianEigenVector, minorEigenVector, eigenValues);
		
		glm::mat3x3 rotMat{ majorEigenVector.x, medianEigenVector.x, minorEigenVector.x,
							majorEigenVector.y, medianEigenVector.y, minorEigenVector.y,
							majorEigenVector.z, medianEigenVector.z, minorEigenVector.z };

		for (Mesh& mesh : model.m_meshes)
			for (glm::vec3& p : mesh.positions)
				p = rotMat * p;
	}

	void GetSortedEigenValues(const Model& model, glm::vec3& eigenValues)
	{
		glm::vec3 eigenVectors[3];
		ComputeEigenVectors(model, eigenVectors[0], eigenVectors[1], eigenVectors[2], eigenValues);
		GetSortedEigenVectors(eigenVectors, eigenVectors[0], eigenVectors[1], eigenVectors[2], eigenValues);
	}

	void GetSortedEigenVectors(const Model& model, glm::vec3& majorEigenVector, glm::vec3& medianEigenVector,
	                           glm::vec3& minorEigenVector)
	{
		glm::vec3 eigenVectors[3];
		glm::vec3 eigenValues;
		ComputeEigenVectors(model, eigenVectors[0], eigenVectors[1], eigenVectors[2], eigenValues);
		GetSortedEigenVectors(eigenVectors, majorEigenVector, medianEigenVector, minorEigenVector, eigenValues);
	}

	void GetSortedEigenVectors(glm::vec3* eigenVectors, glm::vec3& majorEigenVector, glm::vec3& medianEigenVector, glm::vec3& minorEigenVector, const glm::vec3& eValues)
	{
		std::vector<int> sortedIndices = { 0, 1, 2 };
		std::vector<float> sortedEigenValues = { eValues.x, eValues.y, eValues.z };
		std::sort(std::begin(sortedIndices), std::end(sortedIndices), [&](int i1, int i2) { return sortedEigenValues[i1] > sortedEigenValues[i2]; });

		majorEigenVector = eigenVectors[sortedIndices[0]];
		medianEigenVector = eigenVectors[sortedIndices[1]];
		minorEigenVector = glm::cross(majorEigenVector, medianEigenVector);
	}
}
