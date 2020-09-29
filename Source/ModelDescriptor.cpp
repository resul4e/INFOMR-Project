#include "ModelDescriptor.h"

#include "FeatureExtraction.h"
#include "ModelUtil.h"

# define M_PI           3.14159265358979323846  /* pi */

void ModelDescriptor::UpdateFeatures()
{
	if (m_model != nullptr)
	{
		m_model->UpdateBounds();
		m_model->CalculateOBB();
	}
	
	m_3DFeatures.surfaceArea = ExtractSurfaceArea(*m_model);
	m_3DFeatures.volume = ExtractVolume(*m_model);
	m_3DFeatures.boundsArea = ExtractAABBArea(*m_model);
	m_3DFeatures.boundsVolume = ExtractAABBVolume(*m_model);
	m_3DFeatures.compactness = (std::pow(M_PI, 1.0 / 3.0) * std::pow((6.0 * m_3DFeatures.volume), 2.0 / 3.0)) / m_3DFeatures.surfaceArea;
	glm::vec3 eigenValues{ 0 };
	util::GetSortedEigenValues(*m_model, eigenValues);
	m_3DFeatures.eccentricity = eigenValues.x / eigenValues.z;

	m_3DFeatures.a1 = ExtractA1(*this);
	ExtractD1(*m_model);
	ExtractD2(*m_model);
	ExtractD3(*m_model);
	ExtractD4(*m_model);
}
