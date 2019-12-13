#include "voxer/scene/Camera.hpp"
#include "voxer/utils.hpp"

using namespace std;

namespace voxer {

auto Camera::serialize() -> string { return ""; }

auto Camera::deserialize(simdjson::ParsedJson::Iterator &pjh) -> Camera {
  if (!pjh.is_object()) {
    throw JSON_error("camera", "object");
  }

  Camera camera{};

  // TODO: maybe not perspective

  if (!pjh.move_to_key("width") || !pjh.is_integer()) {
    throw JSON_error("camera.width", "integer");
  }
  camera.width = pjh.get_integer();
  pjh.up();

  if (!pjh.move_to_key("height") || !pjh.is_integer()) {
    throw JSON_error("camera.height", "integer");
  }
  camera.height = pjh.get_integer();
  pjh.up();

  const array<string, 3> indexes = {"pos", "dir", "up"};
  const array<array<float, 3> *, 3> targets = {&camera.pos, &camera.dir,
                                               &camera.up};
  for (size_t i = 0; i < indexes.size(); i++) {
    string idx = indexes[i];
    if (!pjh.move_to_key(idx.c_str()) || !pjh.is_array()) {
      throw JSON_error("camera." + idx, "array");
    }
    for (size_t j = 0; j < 3; j++) {
      if (!pjh.move_to_index(j) || !is_number(pjh)) {
        throw JSON_error("camera." + idx + "[i]", "number");
      }
      (*targets[i])[j] = get_number(pjh);
      pjh.up();
    }
    pjh.up();
  }

  return camera;
}

} // namespace voxer