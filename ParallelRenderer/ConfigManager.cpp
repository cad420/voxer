#include "ParallelRenderer/ConfigManager.h"
#include "third_party/rapidjson/document.h"
#include <fstream>
#include <string>
#include <iostream>

using namespace std;

void ConfigManager::load(string filepath) {
  this->filepath = filepath;
  ifstream filestream;
  filestream.open(filepath);
  if (!filestream.is_open()) {
    throw string("Unable to open file configures.json!");
  }
  rapidjson::IStreamWrapper isw(filestream);
  configs.ParseStream(isw);
  if (!configs.IsObject()) {
    throw string("Invalid data file!");
  }
  filestream.close();
}

rapidjson::Value &ConfigManager::get(string id) {
  if (!configs.HasMember(id.c_str())) {
    throw string("Not Found");
  }
  return configs[id.c_str()];
}

string ConfigManager::save(rapidjson::Value &params) {
  auto id = UUID.createRandom().toString();
  auto &rendererParams = params["renderer"];
  auto &posParams = rendererParams["pos"];
  auto &dirParams = rendererParams["dir"];
  auto &upParams = rendererParams["up"];
  posParams.AddMember("x", 100, configs.GetAllocator());
  posParams.AddMember("y", 100, configs.GetAllocator());
  posParams.AddMember("z", 100, configs.GetAllocator());
  dirParams.AddMember("x", -1, configs.GetAllocator());
  dirParams.AddMember("y", -1, configs.GetAllocator());
  dirParams.AddMember("z", -1, configs.GetAllocator());
  upParams.AddMember("x", 0, configs.GetAllocator());
  upParams.AddMember("y", -1, configs.GetAllocator());
  upParams.AddMember("z", 0, configs.GetAllocator());
  configs.GetObject().AddMember(
      rapidjson::Value(id.c_str(), configs.GetAllocator()).Move(), params,
      configs.GetAllocator());
  ofstream ofs(filepath);
  rapidjson::OStreamWrapper osw(ofs);
  rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
  configs.Accept(writer);
  return id;
}