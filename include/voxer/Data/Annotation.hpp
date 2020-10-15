#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace voxer {

struct Annotation {
  enum struct Type { Polygon, Rect };

  std::string type;
  std::string comment;

  using Point = std::array<uint32_t, 2>;
  uint32_t label = 0;
  std::vector<Point> coordinates;

  bool has_hole_inside();
};

} // namespace voxer
