#pragma once

#include "Model.h"

#include <filesystem>
#include <memory>

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
	void UpdateFeatures();
	void UpdateBounds();

	std::string m_name;
	std::filesystem::path m_path;

	std::shared_ptr<Model> m_model;

	std::string m_class;
	//size_t m_vertexCount;
	//size_t m_faceCount;

	Features3D m_3DFeatures;
};
