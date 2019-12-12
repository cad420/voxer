#include "voxer/Slice.hpp"
#include "voxer/utils.hpp"

using namespace std;

namespace voxer {

auto Slice::serialize() -> string { return ""; }

auto Slice::deserialize(simdjson::ParsedJson::Iterator &pjh) -> Slice {
  if (!pjh.is_object()) {
    throw JSON_error("slice", "object");
  }

  Slice slice{};

  if (!pjh.move_to_key("coef") || !pjh.is_array()) {
    throw JSON_error("slice.coef", "array");
  }
  for (size_t i = 0; i < 4; i++) {
    if (!pjh.move_to_index(i) || !is_number(pjh)) {
      throw JSON_error("slice.coef[j]", "double");
    }
    slice.coef[i] = get_number(pjh);
  }
  pjh.up();

  if (!pjh.move_to_key("volume") || !pjh.is_integer()) {
    throw JSON_error("slice.volume", "integer");
  }
  slice.volume_idx = pjh.get_integer();
  pjh.up();

  return slice;
}

} // namespace voxer