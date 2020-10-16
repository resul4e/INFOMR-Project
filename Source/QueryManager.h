#pragma once
#include <filesystem>
#include "Database.h"

class Model;

class QueryManager
{
public:
	QueryManager();
	~QueryManager();

	/**
	 * @brief Returns a pointer to the database.
	 * @return The database containing all loaded models.
	*/
	std::shared_ptr<Database> GetDatabase();
private:
	std::shared_ptr<Database> m_database;
};
