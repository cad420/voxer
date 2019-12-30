#pragma once
#include <array>
#include <simdjson/parsedjson.h>
#include <voxer/filter/clip.hpp>
#include <voxer/formatter/formatter.hpp>

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

  auto serialize() -> rapidjson::Document;
  static auto deserialize(const rapidjson::Value &json) -> SceneDataset;
};

} // namespace voxer

namespace formatter {

template <> inline auto registerMembers<voxer::SceneDataset>() {
  using SceneDataset = voxer::SceneDataset;
  return std::make_tuple(member("name", &SceneDataset::name),
                         member("variable", &SceneDataset::variable),
                         member("timestep", &SceneDataset::timestep));
}

} // namespace formatter
