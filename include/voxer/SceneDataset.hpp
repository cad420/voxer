#pragma once
#include <array>
#include <simdjson/parsedjson.h>

namespace voxer {

struct SceneDataset {
  std::string name = "";
  std::string variable = "";
  uint32_t timestep = 0;

  auto serialize() -> std::string;
  static auto deserialize(simdjson::ParsedJson::Iterator &pjh) -> SceneDataset;
};

} // namespace voxer