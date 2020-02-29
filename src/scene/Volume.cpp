#include "voxer/scene/Volume.hpp"
#include "voxer/utils.hpp"
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>

using namespace std;

namespace seria {

template <> inline auto registerObject<voxer::Volume>() {
  using Volume = voxer::Volume;
  return std::make_tuple(member("dataset", &Volume::dataset_idx),
                         member("tfcn", &Volume::tfcn_idx),
                         member("spacing", &Volume::spacing),
                         member("render", &Volume::render));
}

} // namespace seria

namespace voxer {

auto Volume::serialize() -> rapidjson::Document {
  return seria::serialize(*this);
}

auto Volume::deserialize(const rapidjson::Value &json) -> Volume {
  if (!json.IsObject()) {
    throw JSON_error("volume", "object");
  }

  Volume volume{};
  seria::deserialize(volume, json);

  return volume;
}

} // namespace voxer