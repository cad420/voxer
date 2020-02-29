#include "voxer/scene/Slice.hpp"
#include "voxer/utils.hpp"
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>

using namespace std;

namespace seria {

template <> inline auto registerObject<voxer::Slice>() {
  using Slice = voxer::Slice;
  return std::make_tuple(member("coef", &Slice::coef),
                         member("volume", &Slice::volume_idx));
}

} // namespace seria

namespace voxer {

auto Slice::serialize() -> rapidjson::Document {
  return seria::serialize(*this);
}

auto Slice::deserialize(const rapidjson::Value &json) -> Slice {
  if (!json.IsObject()) {
    throw JSON_error("slice", "object");
  }

  Slice slice{};
  seria::deserialize(slice, json);

  return slice;
}

} // namespace voxer