#pragma once

#include "Model.h"
#include "Graphics/Image.h"
#include "Feature.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <memory>
#include <vector>

struct Features3D
{
	Features3D();

	Bounds bounds;
	float volume;
	float surfaceArea;
	float compactness;
	float boundsArea;
	float boundsVolume;
	float eccentricity;

	HistogramFeature a3;
	HistogramFeature d1;
	HistogramFeature d2;
	HistogramFeature d3;
	HistogramFeature d4;

	inline static glm::vec2 globalBoundsA3 = glm::vec2(std::numeric_limits<float>().max(), std::numeric_limits<float>().lowest());
	inline static glm::vec2 globalBoundsD1 = glm::vec2(std::numeric_limits<float>().max(), std::numeric_limits<float>().lowest());
	inline static glm::vec2 globalBoundsD2 = glm::vec2(std::numeric_limits<float>().max(), std::numeric_limits<float>().lowest());
	inline static glm::vec2 globalBoundsD3 = glm::vec2(std::numeric_limits<float>().max(), std::numeric_limits<float>().lowest());
	inline static glm::vec2 globalBoundsD4 = glm::vec2(std::numeric_limits<float>().max(), std::numeric_limits<float>().lowest());
};

struct ModelDescriptor
{
	ModelDescriptor();

	FeatureVector ComputeFeatureVector();
	void UpdateFeatures();
	void UpdateBounds();

	std::string m_name;
	std::filesystem::path m_path;

	std::shared_ptr<Model> m_model;
	std::vector<Image> m_projections;

	///FILTER DATA
	/**
	 * @brief The shape class of the model.
	*/
	std::string m_class;
	size_t m_vertexCount;
	size_t m_faceCount;
	Bounds m_bounds;
	std::vector<glm::vec3> m_eigenVectors;
	glm::vec3 m_eigenValues;

	Features3D m_3DFeatures;
};
