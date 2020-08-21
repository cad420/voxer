#pragma once
#include <array>
#include <rapidjson/document.h>
#include <vector>
#include <voxer/Data/Color.hpp>

namespace voxer {

struct ControlPoint {
  float x = 0.0f;
  float y = 0.0f;
  std::string hex_color = "";
  std::array<float, 3> color = {};
};

struct TransferFunction {
  std::vector<float> opacities;
  std::vector<float> colors;
};

auto interpolate_tfcn(const TransferFunction &tf)
    -> std::pair<std::vector<float>, std::vector<std::array<float, 3>>>;

} // namespace voxer
