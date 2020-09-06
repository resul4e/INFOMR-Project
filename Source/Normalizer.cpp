#include "Normalizer.h"
#include <iostream>

void Normalizer::Normalize(Model& _model)
{
	CenterModel(_model);
	ScaleModel(_model);
	_model.m_isUploaded = false;
}

void Normalizer::CenterModel(Model& _model)
{
	glm::vec3 mean{ 0,0,0 };
	for (int i = 0; i < 3; i++)
	{
		int positionAmounts = 0;
		for(int k = 0; k < _model.m_meshes.size(); k++)
		{
			positionAmounts += _model.m_meshes[k].positions.size();
			for (int j = 0; j < _model.m_meshes[k].positions.size(); j++)
			{
				mean[i] += _model.m_meshes[k].positions[j][i];
			}
		}
		mean[i] /= static_cast<float>(positionAmounts);
	}

	for (int k = 0; k < _model.m_meshes.size(); k++)
	{
		for (int j = 0; j < _model.m_meshes[k].positions.size(); j++)
		{
			_model.m_meshes[k].positions[j] -= mean;
		}
	}
}

void Normalizer::ScaleModel(Model& _model)
{
	float largestDiff = 0;

	for (int l = 0; l < _model.m_meshes.size(); l++)
	{
		for (int i = 0; i < _model.m_meshes[l].positions.size() - 1; i++)
		{
			for (int m = 0; m < _model.m_meshes.size(); m++)
			{
				for (int j = i + 1; j < _model.m_meshes[m].positions.size(); j++)
				{
					for (int k = 0; k < 3; k++)
					{
						float diff = std::abs(_model.m_meshes[l].positions[i].x - _model.m_meshes[m].positions[j].x);
						if (diff > largestDiff)
						{
							largestDiff = diff;
						}
					}
				}
			}
		}
	}

	for (int l = 0; l < _model.m_meshes.size(); l++)
	{
		for (int j = 0; j < _model.m_meshes[l].positions.size(); j++)
		{
			_model.m_meshes[l].positions[j] /= largestDiff;
		}
	}
}
