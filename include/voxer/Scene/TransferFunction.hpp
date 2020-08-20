#pragma once
#include <array>
#include <rapidjson/document.h>
#include <vector>

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

void deserialize_tfcn(TransferFunction &tfcn, const rapidjson::Value &json);

auto interpolate_tfcn(const TransferFunction &tf)
    -> std::pair<std::vector<float>, std::vector<std::array<float, 3>>>;

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
