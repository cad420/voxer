#pragma once
#include <simdjson/jsonparser.h>
#include <vector>
#include <voxer/formatter/formatter.hpp>

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

namespace formatter {

template <> inline auto registerMembers<voxer::TransferFunction>() {
  using TransferFunction = voxer::TransferFunction;
  return std::make_tuple(member("stops", &TransferFunction::stops),
                         member("opacities", &TransferFunction::opacities),
                         member("colors", &TransferFunction::colors));
}

} // namespace formatter
