#include "Feature.h"

#include <cassert>

float EuclideanDistance::distance(const Feature& f1, const Feature& f2)
{
	assert(f1.m_values.size() == f2.m_values.size());
	
	if (f1.m_values.size() == 1)
		return abs(f1.m_values[0] - f2.m_values[0]);

	float sum = 0;
	for (int i = 0; i < f1.m_values.size(); i++)
	{
		float f = (f1.m_values[i] - f2.m_values[i]);
		sum += f * f;
	}
	return sqrt(sum);
}

void FeatureVector::AddFeature(Feature feature, float weight)
{
	m_features.push_back(feature);
	m_weights.push_back(weight);
}
