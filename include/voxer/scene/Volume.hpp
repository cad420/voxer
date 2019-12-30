#pragma once
#include <cstdint>
#include <memory>
#include <simdjson/jsonparser.h>
#include <string>
#include <voxer/Dataset.hpp>
#include <voxer/formatter/formatter.hpp>

namespace voxer {

struct Volume {
  int32_t dataset_idx = -1;
  int32_t tfcn_idx = -1;
  std::array<float, 3> spacing = {1.0f, 1.0f, 1.0f};

  std::array<float, 2> range = {0.0f, 100.0f};
  std::array<float, 3> translate = {0.0f, 0.0f, 0.0f};
  std::array<float, 3> scale = {1.0f, 1.0f, 1.0f};
  std::array<float, 3> clipBoxLower = {0.0f, 0.0f, 0.0f};
  std::array<float, 3> clipBoxUpper = {0.0f, 0.0f, 0.0f};
  bool render = false;

  auto serialize() -> std::string;
  static auto deserialize(simdjson::ParsedJson::Iterator &pjh) -> Volume;
};

} // namespace voxer

namespace formatter {

template <> inline auto registerMembers<voxer::Volume>() {
  using Volume = voxer::Volume;
  return std::make_tuple(
      member("dataset", &Volume::dataset_idx), member("range", &Volume::range),
      member("translate", &Volume::translate), member("scale", &Volume::scale),
      member("spacing", &Volume::spacing));
}

} // namespace formatter
