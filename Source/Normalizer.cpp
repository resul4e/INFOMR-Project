#include "Normalizer.h"
#include <iostream>

void Normalizer::Normalize(Model& _model)
{
	CenterModel(_model);
	_model.m_isUploaded = false;
}

void Normalizer::CenterModel(Model& _model)
{
	glm::vec3 mean{ 0,0,0 };
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < _model.m_meshes[0].positions.size(); j++)
		{
			mean[i] += _model.m_meshes[0].positions[j][i];
		}
		mean[i] /= static_cast<float>(_model.m_meshes[0].positions.size());
	}

	for (int j = 0; j < _model.m_meshes[0].positions.size(); j++)
	{
		_model.m_meshes[0].positions[j] -= mean;
	}
}
