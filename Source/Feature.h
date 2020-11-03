#pragma once

#include <vector>

class Feature
{
public:
	Feature(int _numValues)
	{
		m_values.resize(_numValues);
	}

	void resize(int _numValues, float _defaultValue = 0) { m_values.resize(_numValues, _defaultValue); }
	const size_t size() const { return m_values.size(); }

	float& operator[](int i) { return m_values[i]; }
	const float& operator[](int i) const { return m_values[i]; }

private:
	std::vector<float> m_values;
};

class HistogramFeature : public Feature
{
public:
	HistogramFeature(int numBins) :
		Feature(numBins),
		m_numBins(numBins),
		m_min(0),
		m_max(0)
	{ }

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

class WassersteinDistance : public DistanceFunction
{
public:
	float distance(const Feature& f1, const Feature& f2) override;
};

class FeatureVector
{
public:
	void AddFeature(Feature feature, float weight = 1);
	void AddFeature(float feature, float weight = 1);
	void AddFeature(HistogramFeature feature, float weight = 1);
	std::vector<float> AsFloatVector() const;

private:
	std::vector<Feature> m_features;
	std::vector<float> m_weights;
	std::vector<HistogramFeature> m_histogramFeatures;

	friend float FeatureVectorDistance(const FeatureVector& fv1, const FeatureVector& fv2, std::vector<float> hist_weights);
};

float FeatureVectorDistance(const FeatureVector& fv1, const FeatureVector& fv2, std::vector<float> hist_weights);
