#pragma once
#include <filesystem>
#include "Database.h"

class Model;

class QueryManager
{
public:
	QueryManager();
	~QueryManager();

	void LoadLabelledPSB(std::filesystem::path _labelledPSBDirectory);
	void LoadPSB(std::filesystem::path _PSBDirectory);

	std::shared_ptr<Database> GetDatabase();
private:
	std::shared_ptr<Database> m_database;
};