#pragma once
#include "ParallelRenderer/GenerateSciVis.h"
#include "ParallelRenderer/DatasetManager.h"
#include "third_party/rapidjson/document.h"
#include <map>
#include <string>
#include <vector>

class Renderer {
public:
  std::vector<unsigned char>
  render(rapidjson::Value &params, gensv::LoadedVolume &volume,
         std::map<std::string, std::string> *extraParams = nullptr);
};