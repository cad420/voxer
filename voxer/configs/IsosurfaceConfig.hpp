#pragma once
#include "third_party/rapidjson/document.h"
#include <ospray/ospray_cpp.h>
#include <string>

struct IsosurfaceConfig {
  float value;
  std::string volumeId;
  IsosurfaceConfig(const rapidjson::Value &params);
};
typedef struct IsosurfaceConfig IsosurfaceConfig;