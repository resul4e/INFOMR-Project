#pragma once

#include "ModelDescriptor.h"

#include <QObject>
#include <flann/flann.hpp>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

namespace std {
	namespace filesystem {
		class path;
	}
}

class Model;

class Database : public QObject
{
	Q_OBJECT
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

	ModelDescriptor FindModelByName(const std::string& _name);

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

	void BuildANNIndex();
	std::vector<int> FindClosestKNNShapes(ModelDescriptor& md, int k);
	std::vector<int> FindClosestANNShapes(ModelDescriptor& md, int k);

signals:
	void featuresLoaded();

private:
	void ComputeFeatureStandardization(DescriptorName _descriptorName);
	void ComputeFeatureVectors();
	void CompoundHistogramPerClass();
	
	std::shared_ptr<Model> LoadSavedModel(std::filesystem::path _modelFileName);
	
	std::vector<ModelDescriptor> m_modelDatabase;

	Features3D m_singleFeatureAverage;
	Features3D m_singleFeatureStddev;

	/** ANN search index*/
	flann::Index<flann::L2<float>> m_index;
};
