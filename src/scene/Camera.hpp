#pragma once
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>
#include <voxer/scene/Camera.hpp>

namespace seria {

template <> auto registerObject<voxer::Camera>() {
  using Camera = voxer::Camera;
  return std::make_tuple(
      member("width", &Camera::width), member("height", &Camera::height),
      member("pos", &Camera::pos), member("up", &Camera::up),
      member("target", &Camera::target), member("ao", &Camera::enable_ao));
}

} // namespace seria
