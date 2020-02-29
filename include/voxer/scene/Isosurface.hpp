#pragma once
#include <memory>
#include <rapidjson/document.h>
#include <voxer/scene/Volume.hpp>

namespace voxer {

struct Isosurface {
  float value = 0.0f;
  int32_t volume_idx = -1;
  bool render = true;

  auto serialize() -> rapidjson::Document;
  static auto deserialize(const rapidjson::Value &json) -> Isosurface;
};

} // namespace voxer
