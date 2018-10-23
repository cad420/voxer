#pragma once
#include "ospray/ospray_cpp.h"
#include "third_party/rapidjson/document.h"
#include "TransferFunctionConfig.h"
#include "DatasetConfig.h"
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
  std::vector<Range> ranges;
  VolumeConfig(rapidjson::Value &);
};
typedef struct VolumeConfig VolumeConfig;