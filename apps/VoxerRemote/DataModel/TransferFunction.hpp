#pragma once
#include <array>
#include <seria/object.hpp>
#include <string>
#include <vector>
#include <voxer/Data/Color.hpp>

namespace voxer::remote {

struct ControlPoint {
  float x;
  float y;
  std::string color;
};

using TransferFunction = std::vector<ControlPoint>;

} // namespace voxer::remote

namespace seria {

template <> inline auto register_object<voxer::remote::ControlPoint>() {
  using ControlPoint = voxer::remote::ControlPoint;
  return std::make_tuple(member("x", &ControlPoint::x),
                         member("y", &ControlPoint::y),
                         member("color", &ControlPoint::color));
}

} // namespace seria
