#include "voxer/scene/Slice.hpp"
#include "voxer/utils.hpp"

using namespace std;

namespace voxer {

auto Slice::serialize() -> rapidjson::Document {
  return formatter::serialize(*this);
}

auto Slice::deserialize(const rapidjson::Value &json) -> Slice {
  if (!json.IsObject()) {
    throw JSON_error("slice", "object");
  }

  Slice slice{};
  formatter::deserialize(slice, json);

  return slice;
}

} // namespace voxer