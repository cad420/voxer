#pragma once
#include "GenerateSciVis.h"
#include <map>
#include <string>
#include "ospray/ospcommon/box.h"
#include "ospray/ospcommon/vec.h"
#include "ospray/ospray_cpp.h"

typedef std::map<std::string, gensv::LoadedVolume> Datasets;

class DatasetManager {
public:
  void load(std::string filepath);
  gensv::LoadedVolume& get(const char* name);
  inline unsigned int size() {
    return datasets.size();
  }
private:
  Datasets datasets;
};