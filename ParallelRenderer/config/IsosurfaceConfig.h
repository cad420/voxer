#pragma once
#include "ospray/ospray_cpp.h"
#include "third_party/rapidjson/document.h"
#include <string>

struct IsosurfaceConfig {
  unsigned char value;
  std::string volumeId;
  IsosurfaceConfig(const rapidjson::Value &params);
};
typedef struct IsosurfaceConfig IsosurfaceConfig;