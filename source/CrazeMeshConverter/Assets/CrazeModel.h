#pragma once

#include "Geometry/MeshData.h"
#include <vector>

std::vector<std::shared_ptr<Craze::Graphics2::MeshData>> loadModelAssimp(const std::string& fileName);