#pragma once
#include <memory>
#include <rapidjson/document.h>
#include <voxer/Dataset.hpp>

namespace voxer {

struct Isosurface {
  Dataset dataset;
  float value = 0.0f;
  std::string color = "#FF0000";
  bool render = true;

  auto serialize() -> rapidjson::Document;
  static auto deserialize(const rapidjson::Value &json) -> Isosurface;
};

} // namespace voxer
