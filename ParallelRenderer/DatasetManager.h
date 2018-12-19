#pragma once
#include "ospray/ospcommon/box.h"
#include "ospray/ospcommon/vec.h"
#include "ospray/ospray_cpp.h"
#include <map>
#include <string>

struct Dataset {
  std::vector<unsigned char> buffer;
  ospcommon::vec3i dimensions;
  ospray::cpp::Data *data;
  std::string dtype;
  size_t sizeForDType;
  std::vector<unsigned int> histogram;
  Dataset() = default;
  Dataset(ospcommon::vec3i _d, std::string dtype)
      : dimensions(_d), dtype(dtype) {
    if (dtype == "uchar" || dtype == "char") {
      sizeForDType = 1;
    } else if (dtype == "float") {
      sizeForDType = 4;
    } else if (dtype == "double") {
      sizeForDType = 8;
    }
    buffer.resize(_d.x * _d.y * _d.z * sizeForDType);
  }
};
typedef struct Dataset Dataset;
typedef std::map<std::string, Dataset> Datasets;

class DatasetManager {
public:
  void load(std::string filepath);
  void add(std::string, Dataset &&);
  Dataset &get(const std::string name);
  inline unsigned int size() { return datasets.size(); }
  inline bool has(std::string name) {
    return datasets.find(name) != datasets.end();
  }

private:
  Datasets datasets;
};