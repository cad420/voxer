#pragma once
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>
#include <voxer/scene/TransferFunction.hpp>

namespace seria {

template <> inline auto registerObject<voxer::ControlPoint>() {
  using ControlPoint = voxer::ControlPoint;
  return std::make_tuple(member("x", &ControlPoint::x),
                         member("y", &ControlPoint::y),
                         member("color", &ControlPoint::hex_color));
}

} // namespace seria
