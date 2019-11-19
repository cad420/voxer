#pragma once
#include <ospray/ospray_cpp.h>
#include <string>

namespace voxer {
struct DatasetConfig {
  std::string name;
  ospcommon::vec3i dimensions;
  ospcommon::vec3f clipingBoxLower;
  ospcommon::vec3f clipingBoxUpper;
};
}
