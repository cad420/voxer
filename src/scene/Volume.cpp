#include "voxer/scene/Volume.hpp"
#include "voxer/utils.hpp"

using namespace std;

namespace voxer {

auto Volume::serialize() -> string { return ""; }

auto Volume::deserialize(simdjson::ParsedJson::Iterator &pjh) -> Volume {
  if (!pjh.is_object()) {
    throw JSON_error("volume", "object");
  }

  Volume volume{};

  if (!pjh.move_to_key("dataset") || !pjh.is_integer()) {
    throw JSON_error("volume.dataset", "integer");
  }
  volume.dataset_idx = pjh.get_integer();
  pjh.up();

  if (!pjh.move_to_key("tfcn") || !pjh.is_integer()) {
    throw JSON_error("volume.tfcn", "integer");
  }
  volume.tfcn_idx = pjh.get_integer();
  pjh.up();

  if (pjh.move_to_key("spacing")) {
    if (!pjh.is_array()) {
      throw JSON_error("volume.spacing", "array");
    }
    for (size_t i = 0; i < 3; i++) {
      if (!pjh.move_to_index(0) || !is_number(pjh)) {
        throw JSON_error("volume.spacing[j]", "number");
      }
      volume.spacing[i] = get_number(pjh);
      pjh.up();
    }
    pjh.up();
  }

  if (!pjh.move_to_key("render") || (!pjh.is_false() && !pjh.is_true())) {
    throw JSON_error("volume.render", "bool");
  }
  volume.render = pjh.is_true();
  pjh.up();

  return volume;
}

} // namespace voxer