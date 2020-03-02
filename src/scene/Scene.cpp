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

  // handle datasets

  return scene;
}

} // namespace voxer