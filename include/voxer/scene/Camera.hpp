#pragma once
#include <array>
#include <map>
#include <simdjson/jsonparser.h>
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
  std::array<float, 3> dir = {0.0f, 0.0f, -1.0f};

  auto serialize() -> std::string;
  static auto deserialize(simdjson::ParsedJson::Iterator &pjh) -> Camera;
};

} // namespace voxer
