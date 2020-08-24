#pragma once
#include <array>

namespace voxer {

struct RGBAColor {
  std::array<float, 4> data;

  void from_hex(const char *hex_color);
};

struct RGBColor {
  std::array<float, 3> data{0, 0, 0};

  void from_hex(const char *hex_color);
};

} // namespace voxer