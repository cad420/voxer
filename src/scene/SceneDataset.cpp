#include "voxer/scene/SceneDataset.hpp"
#include "voxer/utils.hpp"
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>

using namespace std;

namespace seria {

template <> inline auto registerObject<voxer::SceneDataset>() {
  using SceneDataset = voxer::SceneDataset;
  return std::make_tuple(member("name", &SceneDataset::name),
                         member("variable", &SceneDataset::variable),
                         member("timestep", &SceneDataset::timestep));
}

} // namespace seria

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