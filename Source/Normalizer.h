#pragma once
#include "Model.h"

class Normalizer
{
public:
	Normalizer() = delete;
	~Normalizer() = delete;

	/**
	 * @brief Performs normalization on a model. This centers the model, aligns it to its eigenvectors, flips the model and scales it to a unit cube.
	 * @param _model The model to perform the normalization on.
	*/
	static void Normalize(Model& _model);

	static void Remesh(Model& _model);
	
private:
	static void CenterModel(Model& _model);
	static void ScaleModel(Model& _model);
};
