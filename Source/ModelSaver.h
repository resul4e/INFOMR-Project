#pragma once

#include <filesystem>
#include <memory>
#include <map>

class Model;

/**
 * \brief Class that saves a model
 */
class ModelSaver
{
public:

	/**
	 * \brief Saves the model in ply format to the given file path.
	 * \param _model The mode to be saved
	 * \param _filePath The filepath to save the model to. The m_path property will be changed to this path in _model 
	 */
	static void SavePly(Model& _model, std::filesystem::path _filePath);
};
