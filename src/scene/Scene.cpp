#include "Scene.hpp"
#include <voxer/utils.hpp>

using namespace std;

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

  // handle tfcns
  for (auto &tfcn : scene.tfcns) {
    for (auto &point: tfcn) {
      point.color = hex_color_to_float(point.hex_color);
    }
  }

  // handle datasets

  return scene;
}

} // namespace voxer