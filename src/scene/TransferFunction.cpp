#include "voxer/scene/TransferFunction.hpp"
#include "voxer/utils.hpp"

using namespace std;

namespace voxer {

auto TransferFunction::serialize() -> string { return ""; }

auto TransferFunction::deserialize(simdjson::ParsedJson::Iterator &pjh)
    -> TransferFunction {
  if (!pjh.is_array()) {
    throw JSON_error("tfcn", "array");
  }
  pjh.down();

  TransferFunction tfcn{};
  do {
    if (!pjh.is_object()) {
      throw JSON_error("tfcn[i]", "object");
    }

    if (!pjh.move_to_key("x") || !is_number(pjh)) {
      throw JSON_error("tfcn[i].x", "double");
    }
    tfcn.stops.emplace_back(get_number(pjh));
    pjh.up();

    if (!pjh.move_to_key("y") || !is_number(pjh)) {
      throw JSON_error("tfcn[i].y", "double");
    }
    tfcn.opacities.emplace_back(get_number(pjh));
    pjh.up();

    if (!pjh.move_to_key("color") || !pjh.is_string()) {
      throw JSON_error("tfcn[i].color", "double");
    }
    tfcn.colors.emplace_back(hex_color_to_float(pjh.get_string()));
    pjh.up();
  } while (pjh.next());
  pjh.up();

  return tfcn;
}

} // namespace voxer