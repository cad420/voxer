#pragma once
#include <memory>
#include <voxer/scene/Volume.hpp>

namespace voxer {

struct Isosurface {
  float value = 0.0f;
  int32_t volume_idx = -1;

  auto serialize() -> std::string;
  static auto deserialize(simdjson::ParsedJson::Iterator &pjh) -> Isosurface;
};

} // namespace voxer
