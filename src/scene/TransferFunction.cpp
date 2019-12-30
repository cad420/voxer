#include "voxer/scene/TransferFunction.hpp"
#include "voxer/utils.hpp"

using namespace std;

namespace voxer {

auto ControlPoint::serialize() -> rapidjson::Document {
  return formatter::serialize(*this);
}

auto ControlPoint::deserialize(const rapidjson::Value &json) -> ControlPoint {
  if (!json.IsArray()) {
    throw JSON_error("tfcn", "array");
  }

  ControlPoint point{};
  formatter::deserialize(point, json);
  point.color = hex_color_to_float(point.hex_color);

  return point;
}

} // namespace voxer