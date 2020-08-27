#pragma once
#include <array>
#include <vector>
#include <voxer/Data/Color.hpp>

namespace voxer {

struct TransferFunction {
  std::vector<float> opacities;
  std::vector<std::array<float, 3>> colors;
};

} // namespace voxer
