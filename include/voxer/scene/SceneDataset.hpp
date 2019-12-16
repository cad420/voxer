#pragma once
#include <array>
#include <simdjson/parsedjson.h>
#include <voxer/filter/clip.hpp>

namespace voxer {

struct SceneDataset {
  std::string name = "";
  std::string variable = "";
  uint32_t timestep = 0;

  int32_t parent = -1;

  bool clip = false;
  ClipBox clip_box;

  bool diff = false;
  int32_t another = -1;

  bool transform = false;
  std::array<float, 16> matrix;

  auto serialize() -> std::string;
  static auto deserialize(simdjson::ParsedJson::Iterator &pjh) -> SceneDataset;
};

} // namespace voxer