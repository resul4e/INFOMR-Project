#include "Feature.h"

#include <cassert>

float EuclideanDistance::distance(const Feature& f1, const Feature& f2)
{
	assert(f1.size() == f2.size());
	
	if (f1.size() == 1)
		return abs(f1[0] - f2[0]);

	float sum = 0;
	for (int i = 0; i < f1.size(); i++)
	{
		float f = (f1[i] - f2[i]);
		sum += f * f;
	}
	return sqrt(sum);
}

void FeatureVector::AddFeature(Feature feature, float weight)
{
	m_features.push_back(feature);
	m_weights.push_back(weight);
}

void FeatureVector::AddFeature(float feature, float weight)
{
	Feature f(1);
	f[0] = feature;
	AddFeature(f, weight);
}

float FeatureVectorSimilarity(const FeatureVector& fv1, const FeatureVector& fv2)
{
	return 0;
}
