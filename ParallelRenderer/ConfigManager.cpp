#include "ParallelRenderer/ConfigManager.h"
#include "third_party/rapidjson/document.h"
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

using namespace std;
using namespace ospcommon;

void ConfigManager::load(string filepath) {
  this->filepath = filepath;
  ifstream filestream;
  filestream.open(filepath);
  if (!filestream.is_open()) {
    throw string("Unable to open file configures.json!");
  }
  rapidjson::IStreamWrapper isw(filestream);
  this->document.ParseStream(isw);
  if (!this->document.IsObject()) {
    throw string("Invalid data file!");
  }

  auto paramsMap = this->document.GetObject();
  for (auto &params : paramsMap) {
    this->configs.emplace(params.name.GetString(), this->create(params.value));
  }
  filestream.close();
}

Config &ConfigManager::get(const string id) {
  if (configs.find(id) == configs.end()) {
    throw string("Not Found");
  }
  return configs.at(id);
}

string ConfigManager::save(rapidjson::Value &params) {
  auto id = UUID.createRandom().toString();
  auto &rendererParams = params["renderer"];
  // auto &posParams = rendererParams["pos"];
  // auto &dirParams = rendererParams["dir"];
  // auto &upParams = rendererParams["up"];
  auto &d = this->document;
  // posParams.AddMember("x", 100, d.GetAllocator());
  // posParams.AddMember("y", 100, d.GetAllocator());
  // posParams.AddMember("z", 100, d.GetAllocator());
  // dirParams.AddMember("x", -1, d.GetAllocator());
  // dirParams.AddMember("y", -1, d.GetAllocator());
  // dirParams.AddMember("z", -1, d.GetAllocator());
  // upParams.AddMember("x", 0, d.GetAllocator());
  // upParams.AddMember("y", -1, d.GetAllocator());
  // upParams.AddMember("z", 0, d.GetAllocator());

  configs[id] = this->create(params);
  d.GetObject().AddMember(rapidjson::Value(id.c_str(), d.GetAllocator()).Move(),
                          params, d.GetAllocator());
  ofstream ofs(filepath);
  rapidjson::OStreamWrapper osw(ofs);
  rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
  d.Accept(writer);
  return id;
}

Config ConfigManager::create(rapidjson::Value &params) {
  Config config;

  if (!params.HasMember("volumes") || !params["volumes"].IsArray()) {
    throw "config should have propery `volumes` with arrary type!";
  }

  auto &volumesParams = params["volumes"];
  for (auto &volumeParams : volumesParams.GetArray()) {
    config.volumeConfigs.push_back(VolumeConfig(volumeParams));
  }

  auto &rendererParams = params["renderer"];

  config.size = vec2i(rendererParams["width"].GetInt(),
                      rendererParams["height"].GetInt());

  auto &cameraParams = rendererParams["camera"];
  config.cameraConfig = CameraConfig(cameraParams);

  auto &volumesToRenderParams = rendererParams["volumes"];

  for (auto &volumeId : volumesToRenderParams.GetArray()) {
    config.volumesToRender.push_back(volumeId.GetString());
  }

  auto &geometiesParams = rendererParams["geometries"];
  for (auto &geometryParams : geometiesParams.GetArray()) {
    auto &type = geometryParams["type"];
    if (type.GetString() == string("slice")) {
      config.sliceConfigs.push_back(SliceConfig(geometryParams));
    } else if (type.GetString() == string("isosurface")) {
      config.isosurfaceConfigs.push_back(IsosurfaceConfig(geometryParams));
    }
  }

  /*
  vector<LightConfig> lightConfigs;
  auto &lightsParams = rendererParams["lights"];
  for (auto &lightParams : lightsParams.GetArray()) {
    LightConfig lightConfig(lightParams);
    modelConfigs.push_back(ModelConfig(lightConfig));
  }
  */
  return config;
}
