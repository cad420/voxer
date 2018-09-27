#pragma once
#include "ospray/ospray_cpp.h"
#include "third_party/rapidjson/document.h"
#include "TransferFunctionConfig.h"
#include "DatasetConfig.h"
#include <string>

struct VolumeConfig {
  std::string id;
  TransferFunctionConfig tfcnConfig;
  DatasetConfig datasetConfig;
  VolumeConfig(rapidjson::Value &);
};
typedef struct VolumeConfig VolumeConfig;