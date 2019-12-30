#include "voxer/scene/Camera.hpp"
#include "voxer/utils.hpp"

using namespace std;

namespace voxer {

auto Camera::serialize() -> rapidjson::Document {
  return formatter::serialize(*this);
}

auto Camera::deserialize(const rapidjson::Value &json) -> Camera {
  if (!json.IsObject()) {
    throw JSON_error("camera", "object");
  }

  Camera camera{};
  // TODO: maybe not perspective
  formatter::deserialize(camera, json);

  return camera;
}

} // namespace voxer