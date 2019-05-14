#pragma once
#include "third_party/rapidjson/document.h"
#include "voxer/configs/DatasetConfig.hpp"
#include "voxer/configs/TransferFunctionConfig.hpp"
#include <ospray/ospray_cpp.h>
#include <string>

struct Range {
  int start;
  int end;
};
typedef struct Range Range;

struct VolumeConfig {
  std::string id;
  TransferFunctionConfig tfcnConfig;
  DatasetConfig datasetConfig;
  ospcommon::vec3f translate;
  float scale;
  std::vector<Range> ranges;
  ospcommon::vec3f gridSpacing;
  VolumeConfig(rapidjson::Value &);
};
typedef struct VolumeConfig VolumeConfig;