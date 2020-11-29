#pragma once
#include <array>
#include <map>
#include <seria/object.hpp>
#include <string>
#include <voxer/Data/Camera.hpp>

namespace voxer::remote {

using Camera = voxer::Camera;

} // namespace voxer::remote

namespace seria {

template <> inline auto register_object<voxer::remote::Camera>() {
  using Camera = voxer::remote::Camera;
  return std::make_tuple(
      member("width", &Camera::width), member("height", &Camera::height),
      member("pos", &Camera::pos), member("up", &Camera::up),
      member("target", &Camera::target), member("zoom", &Camera::zoom));
}

} // namespace seria
