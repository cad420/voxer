#pragma once
#include <cstdint>
#include <seria/utils.hpp>

namespace voxer::remote {

struct Isosurface {
  float value = 0.0f;
  int32_t dataset_idx = -1;
  std::string color = "#FF0000";
  bool render = true;
};

} // namespace voxer::remote

namespace seria {

template <> inline auto registerObject<voxer::remote::Isosurface>() {
  using Isosurface = voxer::remote::Isosurface;
  return std::make_tuple(member("value", &Isosurface::value),
                         member("dataset", &Isosurface::dataset_idx),
                         member("color", &Isosurface::color),
                         member("render", &Isosurface::render));
}

} // namespace seria
