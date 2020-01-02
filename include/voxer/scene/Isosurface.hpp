#pragma once
#include <memory>
#include <voxer/formatter/formatter.hpp>
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

namespace formatter {

template <> inline auto registerMembers<voxer::Isosurface>() {
  using Isosurface = voxer::Isosurface;
  return std::make_tuple(member("value", &Isosurface::value),
                         member("volume", &Isosurface::volume_idx),
                         member("render", &Isosurface::render));
}

} // namespace formatter
