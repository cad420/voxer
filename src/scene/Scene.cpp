#include "voxer/Scene.hpp"
#include "voxer/utils.hpp"

using namespace std;

namespace voxer {

auto Scene::serialize() -> rapidjson::Document {
  return formatter::serialize(*this);
}

auto Scene::deserialize(const rapidjson::Value &json) -> Scene {
  if (!json.IsObject()) {
    throw JSON_error("scene", "object");
  }

  Scene scene{};

  formatter::deserialize(scene, json);

  return scene;
}

} // namespace voxer