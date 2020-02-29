#include "voxer/scene/Camera.hpp"
#include "voxer/utils.hpp"
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>

using namespace std;

namespace seria {

template <> inline auto registerObject<voxer::Camera>() {
  using Camera = voxer::Camera;
  return std::make_tuple(
      member("width", &Camera::width), member("height", &Camera::height),
      member("pos", &Camera::pos), member("up", &Camera::up),
      member("dir", &Camera::dir), member("ao", &Camera::enable_ao));
}

} // namespace seria

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