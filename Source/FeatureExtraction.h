#pragma once

#include "Model.h"

#include <glm/glm.hpp>
#include <iostream>

float ExtractSurfaceArea(Model& model);
std::vector<double> ExtractFaceAreas(Model& model);
