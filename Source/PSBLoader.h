#pragma once

namespace std {
	namespace filesystem {
		class path;
	}
}

class Database;

namespace io
{
	/**
	 * @brief Given a path to the labelled PSB data loads all models and their shape class.
	 * @param _labelledPSBDirectory The path to the directory containing all labelled model directories
	 * @note When alphabetically sorted the directory should start with "Airplane" "Ant" etc.
	*/
	void LoadLabelledPSB(const std::filesystem::path& _labelledPSBDirectory, Database& _database);
	/**
	 * @brief Give a path to the Princeton Shape Benchmark database loads all models and their shape class.
	 * @param _PSBDirectory The path to the directory containing the classifications directory and the db directory.
	*/
	void LoadPSB(const std::filesystem::path& _PSBDirectory, Database& _database);
}
