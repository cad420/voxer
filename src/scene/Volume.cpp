#include "Volume.hpp"
#include <voxer/utils.hpp>

using namespace std;

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