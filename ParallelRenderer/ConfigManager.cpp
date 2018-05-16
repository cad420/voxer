#include "ParallelRenderer/ConfigManager.h"
#include "third_party/rapidjson/document.h"
#include <fstream>
#include <string>

using namespace std;

void ConfigManager::load(string filepath) {
  ifstream filestream;
  filestream.open("configures.json");
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
  configs.GetObject().AddMember(
      rapidjson::Value(id.c_str(), configs.GetAllocator()).Move(), params,
      configs.GetAllocator());
  ofstream ofs("configures.json");
  rapidjson::OStreamWrapper osw(ofs);
  rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
  configs.Accept(writer);
  return id;
}