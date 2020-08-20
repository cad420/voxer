#pragma once
#include <array>
#include <seria/utils.hpp>
#include <vector>

namespace voxer::remote {

using ControlPoint = voxer::ControlPoint;

using TransferFunction = std::vector<ControlPoint>;

} // namespace voxer::remote

namespace seria {

template <> inline auto registerObject<voxer::remote::ControlPoint>() {
  using ControlPoint = voxer::remote::ControlPoint;
  return std::make_tuple(member("x", &ControlPoint::x),
                         member("y", &ControlPoint::y),
                         member("color", &ControlPoint::hex_color));
}

} // namespace seria