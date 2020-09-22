#pragma once
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
	Database() = default;
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
	void AddModel(std::shared_ptr<Model> _model);

	/**
	 * @brief Given a path to the labelled PSB data loads all models and their shape class.
	 * @param _labelledPSBDirectory The path to the directory containing all labelled model directories
	 * @note When alphabetically sorted the directory should start with "Airplane" "Ant" etc.
	*/
	void LoadLabelledPSB(const std::filesystem::path& _labelledPSBDirectory);
	/**
	 * @brief Give a path to the Princeton Shape Benchmark database loads all models and their shape class.
	 * @param _PSBDirectory The path to the directory containing the classifications directory and the db directory.
	*/
	void LoadPSB(const std::filesystem::path& _PSBDirectory);

	/**
	 * @brief Returns the list of all models.
	 * @return All of the models in this database.
	*/
	std::vector<std::shared_ptr<Model>> GetModelDatabase();

	/**
	 * @brief Goes through each model and subdivides it if it is necessary and normalises it.
	*/
	void ProcessAllModels();
	void NormalizeAllModels();

	/**
	 * @brief Sorts the model database on the given SortingOptions.
	 * @param _option The sorting option, current supports Vertex count, Face count and AABB cross section.
	*/
	void SortDatabase(SortingOptions _option);

private:

	void SubdivideModel(std::shared_ptr<Model>& _model);
	void CrunchModel(std::shared_ptr<Model>& _model);
	
	std::shared_ptr<Model> LoadModifiedModel(std::string _modelFileName);
	
	std::vector<std::shared_ptr<Model>> m_modelDatabase;
};
