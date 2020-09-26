#pragma once

#include "Model.h"

#include <glm/glm.hpp>
#include <iostream>

float ExtractSurfaceArea(Model& model);
float ExtractAABBArea(Model& _model);
std::vector<double> ExtractFaceAreas(Model& model);
