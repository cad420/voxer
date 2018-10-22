#pragma once
#include <string>
#include "ospray/ospray_cpp.h"

struct DatasetConfig {
  std::string name;
  ospcommon::vec3f clipingBoxLower;
  ospcommon::vec3f clipingBoxUpper;
};
typedef struct DatasetConfig DatasetConfig;