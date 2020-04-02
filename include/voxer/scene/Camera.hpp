#pragma once
#include <array>
#include <map>
#include <rapidjson/document.h>
#include <string>

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
  std::array<float, 3> target = {0.0f, 0.0f, 0.0f};
  bool enable_ao = false;

  auto serialize() -> rapidjson::Document;
  static auto deserialize(const rapidjson::Value &json) -> Camera;
};

} // namespace voxer
