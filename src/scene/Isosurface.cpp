#include "voxer/scene/Isosurface.hpp"
#include "voxer/utils.hpp"
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>

using namespace std;

namespace seria {

template <> inline auto registerObject<voxer::Isosurface>() {
  using Isosurface = voxer::Isosurface;
  return std::make_tuple(member("value", &Isosurface::value),
                         member("volume", &Isosurface::volume_idx),
                         member("render", &Isosurface::render));
}

} // namespace seria

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