#pragma once
#include <vector>
#include <voxer/formatter/formatter.hpp>

namespace voxer {

struct ControlPoint {
  float x = 0.0f;
  float y = 0.0f;
  std::string hex_color = "";
  std::array<float, 3> color = {};

  auto serialize() -> rapidjson::Document;
  static auto deserialize(const rapidjson::Value &json) -> ControlPoint;
};

using TransferFunction = std::vector<ControlPoint>;
// struct TransferFunction {
//  std::vector<ControlPoint> points;
//  std::vector<float> stops = {};
//  std::vector<float> opacities = {};
//  std::string hex_color = "";
//  std::vector<std::array<float, 3>> colors = {};
//
//  auto serialize() -> rapidjson::Document;
//  static auto deserialize(const rapidjson::Value &json) -> TransferFunction;
//};

} // namespace voxer

namespace formatter {

template <> inline auto registerMembers<voxer::ControlPoint>() {
  using ControlPoint = voxer::ControlPoint;
  return std::make_tuple(member("x", &ControlPoint::x),
                         member("y", &ControlPoint::y),
                         member("color", &ControlPoint::hex_color));
}
//
// template <> inline auto registerMembers<voxer::TransferFunction>() {
//  using TransferFunction = voxer::TransferFunction;
//  return std::make_tuple(member("points", &TransferFunction::points));
//}

} // namespace formatter
