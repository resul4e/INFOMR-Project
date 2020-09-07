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

private:
	std::vector<std::shared_ptr<Model>> m_modelDatabase;
};
