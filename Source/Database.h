#pragma once
#include <memory>
#include <vector>

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
	void LoadLabelledPSB(std::filesystem::path _labelledPSBDirectory);
	/**
	 * @brief Give a path to the Princeton Shape Benchmark database loads all models and their shape class.
	 * @param _PSBDirectory The path to the directory containing the classifications directory and the db directory.
	*/
	void LoadPSB(std::filesystem::path _PSBDirectory);

	/**
	 * @brief Returns the list of all models.
	 * @return All of the models in this database.
	*/
	std::vector<std::shared_ptr<Model>> GetModelDatabase();

	void ProcessAllModels();

private:
	std::vector<std::shared_ptr<Model>> m_modelDatabase;
};
