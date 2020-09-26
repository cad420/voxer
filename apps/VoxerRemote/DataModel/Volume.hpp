#pragma once
#include "DataModel/StructuredGrid.hpp"
#include <cstdint>
#include <seria/object.hpp>
#include <string>

namespace voxer::remote {

struct Volume {
  DatasetId dataset;
  int32_t tfcn_idx = -1;
  std::array<float, 3> spacing = {1.0f, 1.0f, 1.0f};

  std::array<float, 2> range = {0.0f, 100.0f};
  std::array<float, 3> translate = {0.0f, 0.0f, 0.0f};
  std::array<float, 3> scale = {1.0f, 1.0f, 1.0f};

  bool render = false;
};

} // namespace voxer::remote

namespace seria {

template <> inline auto register_object<voxer::remote::Volume>() {
  using Volume = voxer::remote::Volume;
  return std::make_tuple(
      member("dataset", &Volume::dataset), member("tfcn", &Volume::tfcn_idx),
      member("spacing", &Volume::spacing), member("render", &Volume::render));
}

} // namespace seria