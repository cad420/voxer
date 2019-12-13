#pragma once
#include <simdjson/jsonparser.h>
#include <vector>

namespace voxer {

struct TransferFunction {
  std::vector<float> stops = {};
  std::vector<float> opacities = {};
  std::vector<std::array<float, 3>> colors = {};

  auto serialize() -> std::string;
  static auto deserialize(simdjson::ParsedJson::Iterator &pjh)
      -> TransferFunction;
};

} // namespace voxer
