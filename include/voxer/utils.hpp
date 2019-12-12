#pragma once
#include <simdjson/jsonparser.h>
#include <stdexcept>

namespace voxer {

class JSON_error : public std::runtime_error {
public:
  JSON_error(const std::string &key, const std::string &excepted)
      : std::runtime_error("invalid JSON: " + key + " should be " + excepted +
                           ".") {}
};

inline auto is_number(const simdjson::ParsedJson::Iterator &pjh) -> bool {
  return (pjh.is_double() || pjh.is_integer());
}

inline auto get_number(const simdjson::ParsedJson::Iterator &pjh) -> float {
  if (pjh.is_integer()) {
    return static_cast<float>(pjh.get_integer());
  }
  return static_cast<float>(pjh.get_double());
}

inline auto hex_color_to_float(const std::string &str) -> std::array<float, 3> {
  auto value = std::stoul(str.substr(1), nullptr, 16);

  std::array<float, 3> color = {
      static_cast<float>((value >> 16u) & 0xFFu) / 255.0f,
      static_cast<float>((value >> 8u) & 0xFFu) / 255.0f,
      static_cast<float>(value & 0xFFu) / 255.0f,
  };

  return color;
}

} // namespace voxer
