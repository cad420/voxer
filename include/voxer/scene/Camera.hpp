#pragma once
#include <array>
#include <map>
#include <string>
#include <voxer/formatter/formatter.hpp>

namespace voxer {

struct Camera {
  enum class Type {
    PERSPECTIVE,
  };
  uint32_t width = 0;
  uint32_t height = 0;
  Type type = Type::PERSPECTIVE;
  std::array<float, 3> pos = {0.0f, 0.0f, 0.1f};
  std::array<float, 3> up = {0.0f, 1.0f, 0.0f};
  std::array<float, 3> dir = {0.0f, 0.0f, -1.0f};
  bool enable_ao = false;

  auto serialize() -> rapidjson::Document;
  static auto deserialize(const rapidjson::Value &json) -> Camera;
};

} // namespace voxer

namespace formatter {

template <> inline auto registerMembers<voxer::Camera>() {
  using Camera = voxer::Camera;
  return std::make_tuple(
      member("width", &Camera::width), member("height", &Camera::height),
      member("pos", &Camera::pos), member("up", &Camera::up),
      member("dir", &Camera::dir), member("ao", &Camera::enable_ao));
}

} // namespace formatter
