#include "voxer/scene/SceneDataset.hpp"
#include "voxer/utils.hpp"

using namespace std;

namespace voxer {

auto SceneDataset::serialize() -> string { return ""; }

auto SceneDataset::deserialize(simdjson::ParsedJson::Iterator &pjh)
    -> SceneDataset {
  if (!pjh.is_object()) {
    throw JSON_error("params.dataset[i]", "object");
  }

  SceneDataset dataset{};

  if (!pjh.move_to_key("type") || !pjh.is_string()) {
    throw JSON_error("params.dataset[i].type", "string");
  }

  string type = pjh.get_string();
  pjh.up();
  if (type == "dataset") {
    if (!pjh.move_to_key("name") || !pjh.is_string()) {
      throw JSON_error("params.datasets[i].name", "string");
    }
    dataset.name = pjh.get_string();
    pjh.up();

    if (pjh.move_to_key("variable")) {
      if (!pjh.is_string()) {
        throw JSON_error("params.datasets[i].name", "string");
      }
      dataset.variable = pjh.get_string();
      pjh.up();
    }

    if (pjh.move_to_key("timestep")) {
      if (!pjh.is_integer()) {
        throw JSON_error("params.datasets[i].dataset.name", "string");
      }
      dataset.timestep = pjh.get_integer();
      pjh.up();
    }

    return dataset;
  } else {
    throw runtime_error("not supported dataset type: " + type);
  };
}

} // namespace voxer