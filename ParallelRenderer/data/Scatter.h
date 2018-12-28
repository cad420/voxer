#pragma once
#include "ospray/ospray_cpp.h"
#include <vector>

struct Scatter {
  int points[256][256];
  int max;
};

Scatter createScatter(std::vector<unsigned char> &A, std::vector<unsigned char> &B, ospcommon::vec3i &dimensions);
