#include "TransferFunction.hpp"
#include <voxer/utils.hpp>

using namespace std;

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

auto interpolate_tfcn(const TransferFunction &tf)
    -> std::pair<std::vector<float>, std::vector<std::array<float, 3>>> {
  static const size_t total_samples = 256;
  vector<float> opacities{};
  opacities.reserve(total_samples);
  vector<array<float, 3>> colors{};
  colors.reserve(total_samples);

  for (int32_t i = 0; i < (tf.size() - 1); ++i) {
    auto start_x = tf[i].x;
    auto end_x = tf[i + 1].x;
    auto start_opacity = tf[i].y;
    auto end_opacity = tf[i + 1].y;
    auto start_r = tf[i].color[0];
    auto start_g = tf[i].color[1];
    auto start_b = tf[i].color[2];
    auto end_r = tf[i + 1].color[0];
    auto end_g = tf[i + 1].color[1];
    auto end_b = tf[i + 1].color[2];

    auto samples = static_cast<uint32_t>(total_samples * (end_x - start_x));
    auto delta = 1.0f / static_cast<float>(samples);
    auto step_opacity = delta * (end_opacity - start_opacity);
    auto step_r = delta * (end_r - start_r);
    auto step_g = delta * (end_g - start_g);
    auto step_b = delta * (end_b - start_b);
    for (auto j = 0; j < samples; j++) {
      opacities.emplace_back(start_opacity + j * step_opacity);
      colors.emplace_back(array<float, 3>{
          start_r + j * step_r, start_g + j * step_g, start_b + j * step_b});
    }
  }

  return make_pair(move(opacities), move(colors));
}

} // namespace voxer