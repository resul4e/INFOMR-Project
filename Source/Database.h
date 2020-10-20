#pragma once

#include "ModelDescriptor.h"

#include <flann/flann.hpp>
#include <memory>
#include <vector>
#include <string>

namespace std {
	namespace filesystem {
		class path;
	}
}

class Model;

class Database
{
public:
	Database();
	~Database() = default;

	enum class SortingOptions
	{
		VERTEX_COUNT,
		FACE_COUNT,
		BOUNDS
	};
	
	/**
	 * @brief Adds a model to the database.
	 * @param _model The model to be added.
	*/
	void AddModel(ModelDescriptor _model);

	/**
	 * @brief Returns the list of all models.
	 * @return All of the models in this database.
	*/
	std::vector<ModelDescriptor>& GetModelDatabase();

	/**
	 * @brief Goes through each model and subdivides it if it is necessary and normalises it.
	*/
	void ProcessAllModels();
	void RemeshAllModels();
	void SaveAllModels();
	void NormalizeAllModels();
	
	/**
	 * @brief Sorts the model database on the given SortingOptions.
	 * @param _option The sorting option, current supports Vertex count, Face count and AABB cross section.
	*/
	void SortDatabase(SortingOptions _option);

	FeatureVector ComputeFeatureVector(const ModelDescriptor& md);
	void ComputeFeatureVectors();

	void BuildANNIndex();
	std::vector<int> FindClosestKNNShapes(ModelDescriptor& md, int k);
	std::vector<int> FindClosestANNShapes(ModelDescriptor& md, int k);

private:
	void ComputeFeatureStandardization();

	void CompoundHistogramPerClass();
	
	std::shared_ptr<Model> LoadSavedModel(std::filesystem::path _modelFileName);
	
	std::vector<ModelDescriptor> m_modelDatabase;

	Features3D m_averageFeatures;
	Features3D m_stddevFeatures;
	flann::Index<flann::L2<float>> m_index;
};
