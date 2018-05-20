#pragma once
#include "ParallelRenderer/DatasetManager.h"
#include "ParallelRenderer/GenerateSciVis.h"
#include "third_party/rapidjson/document.h"
#include <map>
#include <string>
#include <vector>

class Renderer {
public:
  std::vector<unsigned char>
  render(rapidjson::Value &params,
         std::map<std::string, std::string> *extraParams = nullptr);
};