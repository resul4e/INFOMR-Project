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
	 * @brief Returns a pointer to the database.
	 * @return The database containing all loaded models.
	*/
	std::shared_ptr<Database> GetDatabase();
private:
	std::shared_ptr<Database> m_database;
};