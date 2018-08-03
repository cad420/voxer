#pragma once
#include "ospray/ospray_cpp.h"
#include "third_party/rapidjson/document.h"
#include <string>
#include <map>

struct CameraConfig {
  std::string type;
  ospcommon::vec3f pos, up, dir;
  CameraConfig(){};
  CameraConfig(const rapidjson::Value &params);
  CameraConfig(const CameraConfig &exist,
               const std::map<std::string, std::string> &params);
  CameraConfig(const CameraConfig &exist, const rapidjson::Value &params);
};
typedef struct CameraConfig CameraConfig;
