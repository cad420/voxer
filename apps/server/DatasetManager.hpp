#pragma once
#include <array>
#include <map>
#include <memory>
#include <simdjson/jsonparser.h>
#include <string>
#include <voxer/Dataset.hpp>

class DatasetManager {
public:
  void load(const std::string &filepath);
  auto get(const std::string &name) -> std::shared_ptr<voxer::Dataset>;
  [[nodiscard]] auto print() const -> std::string;

private:
  simdjson::ParsedJson pj;
  std::map<std::string, std::shared_ptr<voxer::Dataset>> datasets;
};
