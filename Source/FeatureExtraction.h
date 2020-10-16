#pragma once

#include "ModelDescriptor.h"

#include <glm/glm.hpp>
#include <iostream>

float ExtractSurfaceArea(ModelDescriptor& _modelDescriptor);
float ExtractAABBArea(ModelDescriptor& _modelDescriptor);
float ExtractAABBVolume(ModelDescriptor& _modelDescriptor);
std::vector<double> ExtractFaceAreas(const ModelDescriptor& _modelDescriptor);
float ExtractVolume(const ModelDescriptor& _modelDescriptor);
HistogramFeature ExtractA3(const ModelDescriptor& _modelDescriptor);
HistogramFeature ExtractD1(const ModelDescriptor& _modelDescriptor);
HistogramFeature ExtractD2(const ModelDescriptor& _modelDescriptor);
HistogramFeature ExtractD3(const ModelDescriptor& _modelDescriptor);
HistogramFeature ExtractD4(const ModelDescriptor& _modelDescriptor);
