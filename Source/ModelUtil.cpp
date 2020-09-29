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

		eVec1 = glm::vec3(eigenVectors.row(0).x(), eigenVectors.row(0).y(), eigenVectors.row(0).z());
		eVec2 = glm::vec3(eigenVectors.row(1).x(), eigenVectors.row(1).y(), eigenVectors.row(1).z());
		eVec3 = glm::vec3(eigenVectors.row(2).x(), eigenVectors.row(2).y(), eigenVectors.row(2).z());
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

		glm::vec3 majorEigenVector = eigenVectors[sortedIndices[0]];
		glm::vec3 medianEigenVector = eigenVectors[sortedIndices[1]];
		glm::vec3 minorEigenVector = glm::cross(majorEigenVector, medianEigenVector);
		
		for (Mesh& mesh : model.m_meshes)
		{
			for (glm::vec3& p : mesh.positions)
			{
				float x = glm::dot(p, majorEigenVector);
				float y = glm::dot(p, medianEigenVector);
				float z = glm::dot(p, minorEigenVector);

				p = glm::vec3(x, y, z);
			}
		}
	}
}
