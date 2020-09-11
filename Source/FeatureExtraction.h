#pragma once

#include "Model.h"

#include <glm/glm.hpp>
#include <iostream>

float ExtractSurfaceArea(Model& model)
{
	float totalSurfaceArea = 0;
	for (Mesh& mesh : model.m_meshes)
	{
		for (Face& face : mesh.faces)
		{
			const glm::vec3& v0 = mesh.positions[face.indices[0]];
			const glm::vec3& v1 = mesh.positions[face.indices[1]];
			const glm::vec3& v2 = mesh.positions[face.indices[2]];

			glm::vec3 e1 = v1 - v0;
			glm::vec3 e2 = v2 - v0;

			float triangleArea = 0.5 * glm::length(glm::cross(e1, e2));
			totalSurfaceArea += triangleArea;
		}
	}
	std::cout << totalSurfaceArea << std::endl;
	return totalSurfaceArea;
}
