#pragma once
#include <array>
#include <seria/object.hpp>
#include <string>
#include <vector>
#include <voxer/Data/Color.hpp>

namespace voxer::remote {

struct ControlPoint {
  float x;
  float y;
  std::string color;
};

using TransferFunction = std::vector<ControlPoint>;

inline auto interpolate_tfcn(const TransferFunction &points)
    -> std::pair<std::vector<float>, std::vector<std::array<float, 3>>> {
  static const size_t total_samples = 256;
  std::vector<float> opacities{};
  opacities.reserve(total_samples);
  std::vector<std::array<float, 3>> colors{};
  colors.reserve(total_samples);

  for (size_t i = 0; i < (points.size() - 1); ++i) {
    auto start_x = points[i].x;
    auto end_x = points[i + 1].x;
    auto start_opacity = points[i].y;
    auto end_opacity = points[i + 1].y;

    voxer::RGBColor start_color{};
    start_color.from_hex(points[i].color.c_str());
    auto start_r = start_color.data[0];
    auto start_g = start_color.data[1];
    auto start_b = start_color.data[2];

    voxer::RGBColor end_color{};
    end_color.from_hex(points[i].color.c_str());
    auto end_r = end_color.data[0];
    auto end_g = end_color.data[1];
    auto end_b = end_color.data[2];

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

  return make_pair(move(opacities), move(colors));
}

} // namespace voxer::remote

namespace seria {

template <> inline auto register_object<voxer::remote::ControlPoint>() {
  using ControlPoint = voxer::remote::ControlPoint;
  return std::make_tuple(member("x", &ControlPoint::x),
                         member("y", &ControlPoint::y),
                         member("color", &ControlPoint::color));
}

} // namespace seria
