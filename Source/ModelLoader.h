#pragma once

#include <filesystem>
#include <memory>
#include <map>

class Model;
struct Features3D;
class HistogramFeature;

/**
 * \brief Class that is responsible for loading a Model using Assimp.
 */
class ModelLoader
{
public:
	/**
	 * \brief Loads a model from file and returns it.
	 * \param _filePath The file to load the model from
	 * \return The model loaded from the given file path
	 */
	static std::shared_ptr<Model> LoadModel(std::filesystem::path _filePath);
	static Features3D LoadFeatures(std::filesystem::path _filePath);
	static void LoadDescriptorData(std::filesystem::path _filePath, int& o_vertexCount, int& o_faceCount);

private:
	static void LoadHistogramFeature(HistogramFeature& _feature, std::ifstream& _stream);
};
