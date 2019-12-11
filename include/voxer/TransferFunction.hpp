#pragma once
#include <vector>

namespace voxer {

struct TransferFunction {
  std::vector<float> stops = {};
  std::vector<float> opacities = {};
  std::vector<std::array<float, 3>> colors = {};
};

} // namespace voxer
