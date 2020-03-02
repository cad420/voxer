#include "SceneDataset.hpp"
#include <voxer/utils.hpp>

using namespace std;

namespace voxer {

auto SceneDataset::serialize() -> rapidjson::Document {
  return seria::serialize(*this);
}

auto SceneDataset::deserialize(const rapidjson::Value &json) -> SceneDataset {
  if (!json.IsObject()) {
    throw JSON_error("dataset", "object");
  }

  SceneDataset dataset{};
  seria::deserialize(dataset, json);

  return dataset;
}

} // namespace voxer