#pragma once
#include <voxer/Data/Annotation.hpp>

namespace voxer {

auto Annotation::has_hole_inside() -> bool { return coordinates.size() > 1; }

} // namespace voxer