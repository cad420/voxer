#include "TransferFunctionConfig.h"

using namespace std;
using namespace ospcommon;

TransferFunctionConfig::TransferFunctionConfig(const rapidjson::Value &params) {
  this->volumeId = "tmp";
  if (!params.IsArray()) {
    throw "invalid transfer function";
  }

  const auto &points = params.GetArray();

  // TODO: too ugly
  for (auto point = points.Begin(); point != points.End() - 1; ++point) {
    auto &startParams = *point;
    auto &endParams = *(point + 1);
    auto start = startParams["x"].GetFloat();
    auto end = endParams["x"].GetFloat();
    auto startHex = string(startParams["color"].GetString());
    auto startR =
        strtol(startHex.substr(1, 2).c_str(), nullptr, 16) * 1.0f / 255;
    auto startG =
        strtol(startHex.substr(3, 2).c_str(), nullptr, 16) * 1.0f / 255;
    auto startB =
        strtol(startHex.substr(5, 2).c_str(), nullptr, 16) * 1.0f / 255;
    auto endHex = string(endParams["color"].GetString());
    auto endR = strtol(endHex.substr(1, 2).c_str(), nullptr, 16) * 1.0f / 255;
    auto endG = strtol(endHex.substr(3, 2).c_str(), nullptr, 16) * 1.0f / 255;
    auto endB = strtol(endHex.substr(5, 2).c_str(), nullptr, 16) * 1.0f / 255;
    auto startOpa = startParams["y"].GetFloat();
    auto endOpa = endParams["y"].GetFloat();

    auto period = 255 * (end - start);
    auto step = 1.0f / period;
    auto rDiff = (endR - startR) * step;
    auto gDiff = (endG - startG) * step;
    auto bDiff = (endB - startB) * step;
    auto opaDiff = (endOpa - startOpa) * step;
    for (auto j = 0; j < period; j++) {
      this->opacities.push_back(startOpa + j * opaDiff);
      this->colors.push_back(
          vec3f{startR + j * rDiff, startG + j * gDiff, startB + j * bDiff});
    }
  }
}
