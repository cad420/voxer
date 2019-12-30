#include "voxer/scene/Isosurface.hpp"
#include "voxer/utils.hpp"

using namespace std;

namespace voxer {

auto Isosurface::serialize() -> rapidjson::Document {
  return formatter::serialize(*this);
}

auto Isosurface::deserialize(const rapidjson::Value &json) -> Isosurface {
  if (!json.IsObject()) {
    throw JSON_error("isosurface", "object");
  }

  Isosurface isosurface{};
  formatter::deserialize(isosurface, json);
  return isosurface;
}

} // namespace voxer