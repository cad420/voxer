#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace voxer {

struct Annotation {
  enum struct Type { Polygon, Rect };

  //  Type type = Type::Polygon;
  std::string type;
  uint32_t tag = 0;
  std::string comment;
  std::vector<std::vector<uint32_t>> coordinates;

  bool has_hole_inside();
};

} // namespace voxer
