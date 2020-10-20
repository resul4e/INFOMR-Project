#include "Feature.h"

#include <metrics/wasserstein.h>

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

float WassersteinDistance::distance(const Feature& f1, const Feature& f2)
{
	const HistogramFeature& hf1 = (const HistogramFeature&) f1;
	const HistogramFeature& hf2 = (const HistogramFeature&) f2;

	std::vector<float> av(hf1.m_numBins);
	std::vector<float> aw(hf1.m_numBins);
	std::vector<float> bv(hf2.m_numBins);
	std::vector<float> bw(hf2.m_numBins);

	for (int i = 0; i < hf1.m_numBins; i++)
	{
		av[i] = i;
		aw[i] = hf1[i];
	}
	for (int i = 0; i < hf2.m_numBins; i++)
	{
		bv[i] = i;
		bw[i] = hf2[i];
	}

	double dist = wasserstein(av, aw, bv, bw);
	return dist;
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
#include <QDebug>
float FeatureVectorDistance(const FeatureVector& fv1, const FeatureVector& fv2)
{
	EuclideanDistance euclidean;
	
	float dist1 = euclidean.distance(fv1.m_features[0], fv2.m_features[0]);
	float dist2 = euclidean.distance(fv1.m_features[1], fv2.m_features[1]);
	float dist3 = euclidean.distance(fv1.m_features[2], fv2.m_features[2]); qDebug() << "Dist3: " << fv1.m_features[2][0] << fv2.m_features[2][0];
	float dist4 = euclidean.distance(fv1.m_features[3], fv2.m_features[3]);
	float dist5 = euclidean.distance(fv1.m_features[4], fv2.m_features[4]);

	return dist1 + dist2 + dist3 + dist4 + dist5;
}
