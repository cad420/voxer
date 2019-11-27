#pragma once
#include <vector>

namespace voxer {

struct TransferFunctionParams {
  std::vector<float> opacities;
  std::vector<float> colors;
};

} // namespace voxer