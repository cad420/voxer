#pragma once
#include <array>
#include <map>
#include <string>

namespace voxer {
struct Camera {
  enum class Type {
    PERSPECTIVE,
  };
  uint8_t width;
  uint8_t height;
  Type type = Type::PERSPECTIVE;
  std::array<float, 3> pos;
  std::array<float, 3> up;
  std::array<float, 3> dir;
};
} // namespace voxer
