#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace voxer {

struct Annotation {
  enum struct Type { Polygon, Rect };
  std::array<uint32_t, 4> bbox;
  std::string type;
  uint32_t id = 0;
  using Point = std::array<uint32_t, 2>;
  std::string label;
  std::vector<std::vector<Point>> coordinates;

  bool has_hole_inside();
};

} // namespace voxer
