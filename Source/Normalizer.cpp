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
		for (int k = 0; k < _model.m_meshes.size(); k++)
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
	//find the maximum and minimum value for each of the three axes.
	float largestDiff = 0;
	glm::vec3 max{ std::numeric_limits<float>::min() };
	glm::vec3 min{ std::numeric_limits<float>::max() };
	for (int i = 0; i < _model.m_meshes.size(); i++)
	{
		for (int j = 0; j < _model.m_meshes[i].positions.size(); j++)
		{
			for (int k = 0; k < 3; k++)
			{
				max[k] = std::max(max[k], _model.m_meshes[i].positions[j][k]);
				min[k] = std::min(min[k], _model.m_meshes[i].positions[j][k]);
			}
		}
	}

	//find the largest difference in one of the three axes.
	for (int k = 0; k < 3; k++)
	{
		float diff = max[k] - min[k];
		if (diff > largestDiff)
		{
			largestDiff = diff;
		}
	}

	//divide each position by the largest difference.
	for (int l = 0; l < _model.m_meshes.size(); l++)
	{
		for (int j = 0; j < _model.m_meshes[l].positions.size(); j++)
		{
			_model.m_meshes[l].positions[j] /= largestDiff;
		}
	}
}
