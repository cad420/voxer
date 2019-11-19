#pragma once
#include <voxer/Dataset.hpp>
#include <map>
#include <string>

namespace voxer {

using Datasets = std::map<std::string, Dataset>;

class DatasetManager {
public:
  void load(std::string filepath);
  void add(std::string, Dataset &&);
  Dataset &get(const std::string name);
  inline unsigned int size() { return datasets.size(); }
  inline bool has(std::string name) {
    return datasets.find(name) != datasets.end();
  }
  Datasets datasets;
};
}

