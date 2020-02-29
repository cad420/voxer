#include "voxer/Scene.hpp"
#include "voxer/utils.hpp"
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>

using namespace std;

namespace seria {

template <> inline auto registerObject<voxer::Scene>() {
  using Scene = voxer::Scene;
  // TODO: parse lights
  return std::make_tuple(
      member("datasets", &Scene::datasets), member("volumes", &Scene::volumes),
      member("tfcns", &Scene::tfcns),
      member("isosurfaces", &Scene::isosurfaces),
      member("slices", &Scene::slices), member("camera", &Scene::camera));
}

} // namespace seria

namespace voxer {

auto Scene::serialize() -> rapidjson::Document {
  return seria::serialize(*this);
}

auto Scene::deserialize(const rapidjson::Value &json) -> Scene {
  if (!json.IsObject()) {
    throw JSON_error("scene", "object");
  }

  Scene scene{};

  seria::deserialize(scene, json);

  // handle datasets

  return scene;
}

} // namespace voxer