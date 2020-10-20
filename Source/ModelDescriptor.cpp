#include "ModelDescriptor.h"

#include "FeatureExtraction.h"
#include "ModelUtil.h"

# define M_PI           3.14159265358979323846  /* pi */

Features3D::Features3D() :
	bounds(),
	volume(0),
	surfaceArea(0),
	compactness(0),
	boundsArea(0),
	boundsVolume(0),
	eccentricity(0),
	a3(0), d1(0), d2(0), d3(0), d4(0)
{

}

ModelDescriptor::ModelDescriptor() :
	m_vertexCount(0),
	m_faceCount(0),
	m_eigenVectors(3, glm::vec3(0, 0, 0)),
	m_eigenValues(0, 0, 0)
{

}

FeatureVector ModelDescriptor::ComputeFeatureVector()
{
	FeatureVector featureVector;

	featureVector.AddFeature(m_3DFeatures.volume);
	featureVector.AddFeature(m_3DFeatures.surfaceArea);
	featureVector.AddFeature(m_3DFeatures.compactness);
	featureVector.AddFeature(m_3DFeatures.boundsArea);
	featureVector.AddFeature(m_3DFeatures.boundsVolume);
	featureVector.AddFeature(m_3DFeatures.eccentricity);
	featureVector.AddFeature(m_3DFeatures.d1);
	featureVector.AddFeature(m_3DFeatures.d2);
	featureVector.AddFeature(m_3DFeatures.d3);
	featureVector.AddFeature(m_3DFeatures.d4);

	return featureVector;
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
		util::ComputeEigenVectors(*m_model, m_eigenVectors[0], m_eigenVectors[1], m_eigenVectors[2], m_eigenValues);
		m_model->CalculateOBB();

		m_3DFeatures.surfaceArea = ExtractSurfaceArea(*this);
		m_3DFeatures.volume = ExtractVolume(*this);
		m_3DFeatures.boundsArea = ExtractAABBArea(*this);
		m_3DFeatures.boundsVolume = ExtractAABBVolume(*this);
		m_3DFeatures.compactness = (std::pow(M_PI, 1.0 / 3.0) * std::pow((6.0 * m_3DFeatures.volume), 2.0 / 3.0)) / m_3DFeatures.surfaceArea;
		m_3DFeatures.eccentricity = m_eigenValues.x / m_eigenValues.z;

		m_3DFeatures.a3 = ExtractA3(*this);
		m_3DFeatures.d1 = ExtractD1(*this);
		m_3DFeatures.d2 = ExtractD2(*this);
		m_3DFeatures.d3 = ExtractD3(*this);
		m_3DFeatures.d4 = ExtractD4(*this);
	}
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
