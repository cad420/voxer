#pragma once
#include <array>
#include <memory>
#include <rapidjson/document.h>
#include <voxer/scene/Volume.hpp>

namespace voxer {

struct Slice {
  std::array<float, 4> coef = {0.0f, 0.0f, 0.0f, 0.0f};
  int32_t volume_idx = -1;

  auto serialize() -> rapidjson::Document;
  static auto deserialize(const rapidjson::Value &json) -> Slice;
};

} // namespace voxer
