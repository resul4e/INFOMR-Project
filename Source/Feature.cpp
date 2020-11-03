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

void FeatureVector::AddFeature(HistogramFeature feature, float weight)
{
	m_histogramFeatures.push_back(feature);
}

std::vector<float> FeatureVector::AsFloatVector() const
{
	std::vector<float> floatVector;
	for (const Feature& f : m_features)
	{
		for (int i = 0; i < f.size(); i++)
			floatVector.push_back(f[i]);
	}
	for (const HistogramFeature& f : m_histogramFeatures)
	{
		for (int i = 0; i < f.m_numBins; i++)
			floatVector.push_back(f[i]);
	}
	return floatVector;
}

#include <QDebug>
float FeatureVectorDistance(const FeatureVector& fv1, const FeatureVector& fv2, std::vector<float> hist_weights)
{
	EuclideanDistance euclidean;
	WassersteinDistance wasserstein;
	
	float dist1 = euclidean.distance(fv1.m_features[0], fv2.m_features[0]);
	//float dist2 = euclidean.distance(fv1.m_features[1], fv2.m_features[1]);
	
	//float dist2 = euclidean.distance(fv1.m_features[1], fv2.m_features[1]);
	//float dist3 = euclidean.distance(fv1.m_features[2], fv2.m_features[2]);
	//float dist4 = euclidean.distance(fv1.m_features[3], fv2.m_features[3]);
	//float dist5 = euclidean.distance(fv1.m_features[4], fv2.m_features[4]);
	//float dist6 = euclidean.distance(fv1.m_features[5], fv2.m_features[5]);
	float dist7 = wasserstein.distance(fv1.m_histogramFeatures[0], fv2.m_histogramFeatures[0]) * hist_weights[0];
	float dist8 = wasserstein.distance(fv1.m_histogramFeatures[1], fv2.m_histogramFeatures[1]) * hist_weights[1];
	float dist9 = wasserstein.distance(fv1.m_histogramFeatures[2], fv2.m_histogramFeatures[2]) * hist_weights[2];
	float dist10 = wasserstein.distance(fv1.m_histogramFeatures[3], fv2.m_histogramFeatures[3]) * hist_weights[3];
	float dist11 = wasserstein.distance(fv1.m_histogramFeatures[4], fv2.m_histogramFeatures[4]) * hist_weights[4];

	return dist1 * 2 + (dist7 + dist8 + dist9 + dist10 + dist11);
}
