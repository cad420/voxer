#include "Isosurface.hpp"
#include <voxer/utils.hpp>

using namespace std;

namespace voxer {

auto Isosurface::serialize() -> rapidjson::Document {
  return seria::serialize(*this);
}

auto Isosurface::deserialize(const rapidjson::Value &json) -> Isosurface {
  if (!json.IsObject()) {
    throw JSON_error("isosurface", "object");
  }

  Isosurface isosurface{};
  seria::deserialize(isosurface, json);
  return isosurface;
}

} // namespace voxer