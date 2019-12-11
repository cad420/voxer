#pragma once
#include <memory>
#include <voxer/Volume.hpp>

namespace voxer {

struct Isosurface {
  float value = 0.0f;
  int32_t volume_idx = -1;
};

} // namespace voxer
