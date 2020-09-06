#pragma once
#include <memory>
#include "Model.h"

class Normalizer
{
public:
	Normalizer() = delete;
	~Normalizer() = delete;

	static void Normalize(Model& _model);
	
private:
	static void CenterModel(Model& _model);
};