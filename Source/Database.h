#pragma once
#include <memory>
#include <vector>

class Model;

class Database
{
public:
	Database() = default;
	~Database() = default;

	void AddModel(std::shared_ptr<Model> _model);

private:
	std::vector<std::shared_ptr<Model>> m_modelDatabase;
};
