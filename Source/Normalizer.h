#pragma once
#include "Model.h"
#include "ModelDescriptor.h"

class Normalizer
{
public:
	Normalizer() = delete;
	~Normalizer() = delete;

	/**
	 * @brief Performs normalization on a model. This centers the model, aligns it to its eigenvectors, flips the model and scales it to a unit cube.
	 * @param _model The model to perform the normalization on.
	*/
	static void Normalize(ModelDescriptor& _modelDescriptor);

	static void Remesh(ModelDescriptor& _model);

	
	
private:
	static void CenterModel(Model& _model);
	static void ScaleModel(Model& _model);
	static void AlignModel(Model& _model);
	static void FlipModel(Model& _model);
};
