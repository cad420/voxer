#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <voxer/Color.hpp>
#include <voxer/Image.hpp>

namespace voxer {

struct Annotation {
  enum struct Type { Polygon, Rect };
  Type type = Type::Polygon;
  uint32_t tag = 0;
  std::string comment;
  std::vector<std::vector<uint32_t>> coordinates;

  auto has_hole() -> bool;
  auto to_image(const Color &color) -> Image;
};

} // namespace voxer
