#include <cstdint>
#include <iostream>
#include <string>
#include <voxer/Data/TransferFunction.hpp>

using namespace std;

namespace voxer {

void TransferFunction::add_point(float x, float y, std::array<float, 3> color) {
  points.emplace_back(ControlPoint{x, y, color});
  is_valid = false;
}

auto TransferFunction::interpolate()
    -> std::pair<std::vector<float>, std::vector<std::array<float, 3>>> {
  if (is_valid) {
    return make_pair(opacities, colors);
  }

  opacities.clear();
  colors.clear();

  static const size_t total_samples = 256;
  opacities.reserve(total_samples);
  colors.reserve(total_samples);

  for (size_t i = 0; i < (points.size() - 1); ++i) {
    auto start_x = points[i].x;
    auto end_x = points[i + 1].x;
    auto start_opacity = points[i].y;
    auto end_opacity = points[i + 1].y;

    auto start_r = points[i].color[0];
    auto start_g = points[i].color[1];
    auto start_b = points[i].color[2];

    auto end_r = points[i + 1].color[0];
    auto end_g = points[i + 1].color[1];
    auto end_b = points[i + 1].color[2];

    auto samples = static_cast<uint32_t>(total_samples * (end_x - start_x));
    auto delta = 1.0f / static_cast<float>(samples);
    auto step_opacity = delta * (end_opacity - start_opacity);
    auto step_r = delta * (end_r - start_r);
    auto step_g = delta * (end_g - start_g);
    auto step_b = delta * (end_b - start_b);
    for (uint32_t j = 0; j < samples; j++) {
      opacities.emplace_back(start_opacity + j * step_opacity);
      colors.emplace_back(std::array<float, 3>{
          start_r + j * step_r, start_g + j * step_g, start_b + j * step_b});
    }
  }

  is_valid = true;

  return make_pair(opacities, colors);
}

} // namespace voxer