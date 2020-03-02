#include "Camera.hpp"
#include <voxer/utils.hpp>

using namespace std;

namespace voxer {

auto Camera::serialize() -> rapidjson::Document {
  return seria::serialize(*this);
}

auto Camera::deserialize(const rapidjson::Value &json) -> Camera {
  if (!json.IsObject()) {
    throw JSON_error("camera", "object");
  }

  Camera camera{};
  // TODO: maybe not perspective
  seria::deserialize(camera, json);

  return camera;
}

} // namespace voxer