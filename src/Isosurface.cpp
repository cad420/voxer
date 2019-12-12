#include "voxer/Isosurface.hpp"
#include "voxer/utils.hpp"

using namespace std;

namespace voxer {

auto Isosurface::serialize() -> string { return ""; }

auto Isosurface::deserialize(simdjson::ParsedJson::Iterator &pjh)
    -> Isosurface {
  if (!pjh.is_object()) {
    throw JSON_error("isosurface", "object");
  }

  Isosurface isosurface{};

  if (!pjh.move_to_key("value") || !is_number(pjh)) {
    throw JSON_error("isosurface.value", "number");
  }
  isosurface.value = get_number(pjh);
  pjh.up();

  if (!pjh.move_to_key("volume") || !pjh.is_integer()) {
    throw JSON_error("isosurface.volume", "integer");
  }
  isosurface.volume_idx = pjh.get_integer();
  pjh.up();

  return isosurface;
}

} // namespace voxer