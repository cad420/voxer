#include "voxer/Scene.hpp"
#include "voxer/utils.hpp"

using namespace std;

namespace voxer {

auto Scene::serialize() -> string { return ""; }

auto Scene::deserialize(simdjson::ParsedJson::Iterator &pjh) -> Scene {
  if (!pjh.is_object()) {
    throw JSON_error("scene", "object");
  }

  Scene scene{};
  // parse datasets
  if (!pjh.move_to_key("datasets") || !pjh.is_array()) {
    throw JSON_error("params.datasets", "array");
  }
  pjh.down(); // into array
  do {
    scene.datasets.emplace_back(SceneDataset::deserialize(pjh));
  } while (pjh.next());
  pjh.up(); // out of array
  pjh.up(); // back to {}

  // parse transfer functions
  if (!pjh.move_to_key("tfcns") || !pjh.is_array()) {
    throw JSON_error("params.tfcns", "array");
  }
  pjh.down(); // into array
  do {
    scene.tfcns.emplace_back(TransferFunction::deserialize(pjh));
  } while (pjh.next());
  pjh.up(); // out of array
  pjh.up(); // back to {}

  // optional keys

  // parse volumes
  if (!pjh.move_to_key("volumes") || !pjh.is_array()) {
    throw JSON_error("params.volumes", "array");
  }
  pjh.down(); // into array
  do {
    scene.volumes.emplace_back(voxer::Volume::deserialize(pjh));
  } while (pjh.next());
  pjh.up(); // out of array
  pjh.up(); // back to {}

  // parse camera
  if (!pjh.move_to_key("camera")) {
    throw JSON_error("params.camera", "required");
  }
  scene.camera = Camera::deserialize(pjh);
  pjh.up(); // back to {}

  // parse isosurfaces
  if (pjh.move_to_key("isosurfaces")) {
    if (!pjh.is_array()) {
      throw JSON_error("params.isosurfaces", "array");
    }
    pjh.down(); // into array
    do {
      scene.isosurfaces.emplace_back(Isosurface::deserialize(pjh));
    } while (pjh.next());
    pjh.up(); // out of array
    pjh.up(); // back to {}
  }

  // parse slices
  if (pjh.move_to_key("slices")) {
    if (!pjh.is_array()) {
      throw JSON_error("params.slices", "array");
    }
    pjh.down(); // into array
    do {
      scene.slices.emplace_back(Slice::deserialize(pjh));
    } while (pjh.next());
    pjh.up(); // out of array
    pjh.up(); // back to {}
  }

  // TODO: parse lights

  return scene;
}

} // namespace voxer