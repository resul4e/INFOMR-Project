#include "Feature.h"

#include <metrics/wasserstein.h>

#include <cassert>

float ManhattanDistance::distance(const Feature& f1, const Feature& f2)
{
	assert(f1.size() == f2.size());

	if (f1.size() == 1)
		return abs(f1[0] - f2[0]);

	float sum = 0;
	for (int i = 0; i < f1.size(); i++)
	{
		float f = (f1[i] - f2[i]);
		sum += abs(f);
	}
	return sum;
}

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
	
	float singleDists = 0;
	float histDists = 0;

	for (int i = 0; i < fv1.m_features.size(); i++)
	{
		const Feature& f1 = fv1.m_features[i];
		const Feature& f2 = fv2.m_features[i];
		singleDists += euclidean.distance(f1, f2);
	}

	for (int i = 0; i < fv1.m_histogramFeatures.size(); i++)
	{
		const HistogramFeature& f1 = fv1.m_histogramFeatures[i];
		const HistogramFeature& f2 = fv2.m_histogramFeatures[i];
		histDists += wasserstein.distance(f1, f2) * hist_weights[i];
	}

	return singleDists * 2 + histDists;
}
