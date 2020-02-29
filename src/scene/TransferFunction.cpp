#include "voxer/scene/TransferFunction.hpp"
#include "voxer/utils.hpp"
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>

using namespace std;

namespace seria {

template <> inline auto registerObject<voxer::ControlPoint>() {
  using ControlPoint = voxer::ControlPoint;
  return std::make_tuple(member("x", &ControlPoint::x),
                         member("y", &ControlPoint::y),
                         member("color", &ControlPoint::hex_color));
}

} // namespace seria

namespace voxer {

auto ControlPoint::serialize() -> rapidjson::Document {
  return seria::serialize(*this);
}

auto ControlPoint::deserialize(const rapidjson::Value &json) -> ControlPoint {
  if (!json.IsArray()) {
    throw JSON_error("tfcn", "array");
  }

  ControlPoint point{};
  seria::deserialize(point, json);
  point.color = hex_color_to_float(point.hex_color);

  return point;
}

void deserialize_tfcn(TransferFunction &tfcn, const rapidjson::Value &json) {
  seria::deserialize(tfcn, json);
}

} // namespace voxer