#pragma once
#include <string>
#include <voxer/Data/Color.hpp>

namespace {

auto hex_color_to_float(const std::string &str) -> std::array<float, 3> {
  auto value = std::stoul(str.substr(1), nullptr, 16);

  std::array<float, 3> color = {
      static_cast<float>((value >> 16u) & 0xFFu) / 255.0f,
      static_cast<float>((value >> 8u) & 0xFFu) / 255.0f,
      static_cast<float>(value & 0xFFu) / 255.0f,
  };

  return color;
}

} // namespace

namespace voxer {

void RGBAColor::from_hex(const char *hex_color) {
  auto color = hex_color_to_float(hex_color);
  for (size_t i = 0; i < 3; i++) {
    data[i] = color[i];
  }
  data[3] = 1.0f;
}

void RGBColor::from_hex(const char *hex_color) {
  data = hex_color_to_float(hex_color);
}

} // namespace voxer