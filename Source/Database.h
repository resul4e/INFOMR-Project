#pragma once
#include <memory>
#include <vector>

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
	 * @brief Returns the list of all models.
	 * @return All of the models in this database.
	*/
	std::vector<std::shared_ptr<Model>> GetModelDatabase();
	
private:
	std::vector<std::shared_ptr<Model>> m_modelDatabase;
};
