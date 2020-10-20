#pragma once

class ModelDescriptor;

namespace proc
{
	/**
	 * @brief Performs normalization on a model. This centers the model, aligns it to its eigenvectors, flips the model and scales it to a unit cube.
	 * @param _model The model to perform the normalization on.
	*/
	void Normalize(ModelDescriptor& _modelDescriptor);

	void Remesh(ModelDescriptor& _modelDescriptor);

	void SubdivideModel(ModelDescriptor& _modelDescriptor);

	void CrunchModel(ModelDescriptor& _modelDescriptor);
}
