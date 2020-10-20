#pragma once

#include <vector>

class Feature
{
public:
	std::vector<float> m_values;
};

class HistogramFeature : public Feature
{
public:
	HistogramFeature(int numBins) :
		m_numBins(numBins),
		m_min(0),
		m_max(0)
	{
		m_values.resize(numBins, 0);
	}
	int m_numBins;
	float m_min;
	float m_max;
};

class DistanceFunction
{
public:
	virtual float distance(const Feature& f1, const Feature& f2) = 0;
};

class EuclideanDistance : public DistanceFunction
{
public:
	float distance(const Feature& f1, const Feature& f2) override;
};

class FeatureVector
{
public:
	void AddFeature(Feature feature, float weight);

private:
	std::vector<Feature> m_features;
	std::vector<float> m_weights;
};
