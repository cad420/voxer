#pragma once
#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/istreamwrapper.h"
#include "third_party/rapidjson/ostreamwrapper.h"
#include "third_party/rapidjson/writer.h"
#include "voxer/configs/CameraConfig.hpp"
#include "voxer/configs/IsosurfaceConfig.hpp"
#include "voxer/configs/SliceConfig.hpp"
#include "voxer/configs/TransferFunctionConfig.hpp"
#include "voxer/configs/VolumeConfig.hpp"
#include <map>
#include <ospray/ospray_cpp.h>
#include <string>
#include <vector>
#include <voxer/Config.hpp>

namespace voxer {

class ConfigManager {
public:
  void load(std::string filepath);
  Config &get(const std::string);
  std::string save(rapidjson::Value &);
  Config create(rapidjson::Value &);

private:
  std::map<std::string, Config> configs;
  rapidjson::Document document;
  std::string filepath;
//  Poco::UUIDGenerator UUID;
};
}
