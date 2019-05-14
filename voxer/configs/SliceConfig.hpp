#pragma once
#include "third_party/rapidjson/document.h"
#include <ospray/ospray_cpp.h>
#include <string>

struct SliceConfig {
  float a;
  float b;
  float c;
  float d;
  std::string volumeId;
  SliceConfig(const rapidjson::Value &params);
};
typedef struct SliceConfig SliceConfig;