#include "FeatureExtraction.h"

namespace
{
	float ExtractFaceArea(const Mesh& mesh, const Face& face)
	{
		const glm::vec3& v0 = mesh.positions[face.indices[0]];
		const glm::vec3& v1 = mesh.positions[face.indices[1]];
		const glm::vec3& v2 = mesh.positions[face.indices[2]];

		glm::vec3 e1 = v1 - v0;
		glm::vec3 e2 = v2 - v0;

		float triangleArea = 0.5f * glm::length(glm::cross(e1, e2));
		return triangleArea;
	}
}

float ExtractSurfaceArea(Model& model)
{
	float totalSurfaceArea = 0;

	for (Mesh& mesh : model.m_meshes)
		for (Face& face : mesh.faces)
			totalSurfaceArea += ExtractFaceArea(mesh, face);

	return totalSurfaceArea;
}

std::vector<double> ExtractFaceAreas(Model& model)
{
	std::vector<double> facesAreas;
	for (Mesh& mesh : model.m_meshes)
	{
		for (Face& face : mesh.faces)
		{
			double triangleArea = ExtractFaceArea(mesh, face);
			facesAreas.push_back(triangleArea);
		}
	}
	return facesAreas;
}

float ExtractAABBArea(Model& _model)
{
	glm::vec3 AABB = _model.m_bounds.max - _model.m_bounds.min;

	return 2 * (AABB.x * AABB.y + AABB.x * AABB.z + AABB.y * AABB.z);
}


float ExtractAABBVolume(Model& _model)
{
	glm::vec3 AABB = _model.m_bounds.max - _model.m_bounds.min;

	return AABB.x * AABB.y * AABB.z;
}

float SignedVolumeOfTriangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
	float v321 = p3.x * p2.y * p1.z;
	float v231 = p2.x * p3.y * p1.z;
	float v312 = p3.x * p1.y * p2.z;
	float v132 = p1.x * p3.y * p2.z;
	float v213 = p2.x * p1.y * p3.z;
	float v123 = p1.x * p2.y * p3.z;
	return (1.0f / 6.0f) * (-v321 + v231 + v312 - v132 - v213 + v123);
}

float ExtractVolumeOfMesh(const Mesh& _mesh) {
	float vols = 0;

	for(int i = 0; i < _mesh.faces.size(); i++)
	{
		const unsigned* indices = _mesh.faces[i].indices;  
		vols += SignedVolumeOfTriangle(_mesh.positions[indices[0]], _mesh.positions[indices[1]], _mesh.positions[indices[2]]);
	}
	
	return std::abs(vols);
}

float ExtractVolume(const Model& _model)
{
	float vol = 0;
	for(const Mesh& mesh : _model.m_meshes)
	{
		vol += ExtractVolumeOfMesh(mesh);
	}
	return vol;
}