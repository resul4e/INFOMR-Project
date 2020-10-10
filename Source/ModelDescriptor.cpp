#include "ModelDescriptor.h"

#include "FeatureExtraction.h"
#include "ModelUtil.h"

# define M_PI           3.14159265358979323846  /* pi */

ModelDescriptor::ModelDescriptor() :
	m_vertexCount(0),
	m_faceCount(0)
{

}

void ModelDescriptor::UpdateFeatures()
{
	if (m_model != nullptr)
	{
		m_vertexCount = 0;
		m_faceCount = 0;
		for (Mesh& mesh : m_model->m_meshes)
		{
			m_vertexCount += mesh.positions.size();
			m_faceCount += mesh.faces.size();
		}

		UpdateBounds();
		m_model->CalculateOBB();
	}
	
	m_3DFeatures.surfaceArea = ExtractSurfaceArea(*this);
	m_3DFeatures.volume = ExtractVolume(*this);
	m_3DFeatures.boundsArea = ExtractAABBArea(*this);
	m_3DFeatures.boundsVolume = ExtractAABBVolume(*this);
	m_3DFeatures.compactness = (std::pow(M_PI, 1.0 / 3.0) * std::pow((6.0 * m_3DFeatures.volume), 2.0 / 3.0)) / m_3DFeatures.surfaceArea;
	glm::vec3 eigenValues{ 0 };
	util::GetSortedEigenValues(*m_model, eigenValues);
	m_3DFeatures.eccentricity = eigenValues.x / eigenValues.z;

	m_3DFeatures.a1 = ExtractA1(*this);
	m_3DFeatures.d1 = ExtractD1(*this);
	m_3DFeatures.d2 = ExtractD2(*this);
	m_3DFeatures.d3 = ExtractD3(*this);
	m_3DFeatures.d4 = ExtractD4(*this);
}

void ModelDescriptor::UpdateBounds()
{
	m_bounds.max = glm::vec3(0, 0, 0);
	m_bounds.min = glm::vec3(0, 0, 0);
	for (int k = 0; k < m_model->m_meshes.size(); k++)
	{
		for (int j = 0; j < m_model->m_meshes[k].positions.size(); j++)
		{
			glm::vec3 v = m_model->m_meshes[k].positions[j];
			for (int d = 0; d < 3; d++)
			{
				m_bounds.min[d] = v[d] < m_bounds.min[d] ? v[d] : m_bounds.min[d];
				m_bounds.max[d] = v[d] > m_bounds.max[d] ? v[d] : m_bounds.max[d];
			}
		}
	}
}
