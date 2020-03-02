#include "Slice.hpp"
#include <voxer/utils.hpp>

using namespace std;

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