#include "voxer/scene/Volume.hpp"
#include "voxer/utils.hpp"

using namespace std;

namespace voxer {

auto Volume::serialize() -> rapidjson::Document {
  return formatter::serialize(*this);
}

auto Volume::deserialize(const rapidjson::Value &json) -> Volume {
  if (!json.IsObject()) {
    throw JSON_error("volume", "object");
  }

  Volume volume{};
  formatter::deserialize(volume, json);

  return volume;
}

} // namespace voxer