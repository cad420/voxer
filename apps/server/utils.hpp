#pragma once
#include "DatasetStore.hpp"
#include <fmt/core.h>
#include <fmt/format.h>
#include <stdexcept>
#include <string_view>
#include <voxer/Dataset.hpp>

class JSON_error : public std::runtime_error {
public:
  JSON_error(const std::string &key, const std::string &excepted)
      : std::runtime_error("invalid JSON: " + key + " should be " + excepted +
                           ".") {}
};

inline auto hex_color_to_float(const std::string &str) -> std::array<float, 3> {
  auto value = std::stoul(str.substr(1), nullptr, 16);

  std::array<float, 3> color = {
      static_cast<float>((value >> 16) & 0xFF) / 255.0f,
      static_cast<float>((value >> 8) & 0xFF) / 255.0f,
      static_cast<float>(value & 0xFF) / 255.0f,
  };

  return color;
}