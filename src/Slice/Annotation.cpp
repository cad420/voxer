#pragma once
#include <voxer/Annotation.hpp>

namespace voxer {

auto Annotation::has_hole() -> bool { return coordinates.size() > 1; }

auto Annotation::to_image(const Color &color) -> Image {
  // TODO
  return Image{};
}

} // namespace voxer