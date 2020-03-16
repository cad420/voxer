#pragma once
#include <array>
#include <rapidjson/document.h>
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

  bool operator==(const SceneDataset &other) const {
    return (name == other.name && variable == other.variable &&
            timestep == other.timestep);
  }

  auto serialize() -> rapidjson::Document;
  static auto deserialize(const rapidjson::Value &json) -> SceneDataset;
};

struct SceneDatasetHasher {
  std::size_t operator()(const SceneDataset &k) const {
    using std::hash;
    using std::size_t;
    using std::string;

    // Compute individual hash values for id, variable and timestep
    // http://stackoverflow.com/a/1646913/126995
    size_t res = 17;
    res = res * 31 + hash<string>()(k.name);
    res = res * 31 + hash<string>()(k.variable);
    res = res * 31 + hash<uint32_t>()(k.timestep);
    return res;
  }
};

} // namespace voxer
