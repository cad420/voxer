#pragma once
#include <vector>

namespace voxer {

struct ControlPoint {
  float stop = 0.0f; // percent
  float opacity = 0.0f;
  std::array<float, 3> color = {0.0f, 0.0f, 0.0f};
};

struct TransferFunction {
  std::vector<ControlPoint> points;
};

} // namespace voxer
