#pragma once
#include <array>

namespace voxer {

struct ClipBox {
  std::array<float, 3> upper = {0.0f, 0.0f, 0.0f};
  std::array<float, 3> lower = {0.0f, 0.0f, 0.0f};
};

} // namespace voxer