#pragma once

#include "Model.h"

#include <filesystem>
#include <memory>

constexpr size_t HISTOGRAM_BIN_SIZE = 10;
struct HistogramFeature
{
	int binCount[HISTOGRAM_BIN_SIZE];
	float max;
	float min;
};

struct Features3D
{
	Bounds bounds;
	float volume;
	float surfaceArea;
	float compactness;
	float boundsArea;
	float boundsVolume;
	float eccentricity;

	HistogramFeature a1;
};

struct ModelDescriptor
{
	ModelDescriptor();

	void UpdateFeatures();
	void UpdateBounds();

	std::string m_name;
	std::filesystem::path m_path;

	std::shared_ptr<Model> m_model;

	std::string m_class;
	size_t m_vertexCount;
	size_t m_faceCount;
	Bounds m_bounds;

	Features3D m_3DFeatures;
};
