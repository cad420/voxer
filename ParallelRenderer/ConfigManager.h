#pragma once
#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/istreamwrapper.h"
#include "third_party/rapidjson/ostreamwrapper.h"
#include "third_party/rapidjson/writer.h"
#include <string>
#include "Poco/UUIDGenerator.h"

class ConfigManager {
public:
  void load(std::string filepath);
  rapidjson::Value& get(std::string);
  std::string save(rapidjson::Value &);

private:
  rapidjson::Document configs;
  Poco::UUIDGenerator UUID;
};