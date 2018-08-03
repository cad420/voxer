#pragma once
#include "ospray/ospray_cpp.h"
#include "third_party/rapidjson/document.h"
#include <map>
#include <string>
#include <vector>

struct TransferFunctionConfig {
  std::string volumeId;
  std::vector<ospcommon::vec3f> colors;
  std::vector<float> opacities;
  TransferFunctionConfig(){};
  TransferFunctionConfig(const rapidjson::Value &params);
};
typedef struct TransferFunctionConfig TransferFunctionConfig;
