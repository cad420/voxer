#include "databases/MRC/MRCReader.hpp"
#include "databases/Raw/RawReader.hpp"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <seria/serialize.hpp>
#include <voxer/Dataset.hpp>
#include <voxer/utils.hpp>

using namespace std;

namespace seria {

template <> inline auto registerObject<voxer::Dataset>() {
  using Dataset = voxer::Dataset;
  return std::make_tuple(member("name", &Dataset::name),
                         member("variable", &Dataset::variable),
                         member("timestep", &Dataset::timestep),
                         member("histogram", &Dataset::histogram));
}

} // namespace seria

namespace voxer {

auto Dataset::serialize() const -> string {
  auto json = seria::serialize(*this);
  rapidjson::StringBuffer rapidjson_buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(rapidjson_buffer);
  json.Accept(writer);
  return rapidjson_buffer.GetString();
}

auto Dataset::get_slice(Axis axis, uint32_t slice) const -> Image {
  Image result{};
  result.channels = 1;
  switch (axis) {
  case Axis::X: {
    if (slice >= info.dimensions[0]) {
      throw runtime_error("slice idx overflow");
    }
    result.width = info.dimensions[1];
    result.height = info.dimensions[2];
    result.data.reserve(result.width * result.height * sizeof(uint8_t));
    for (int z = 0; z < info.dimensions[2]; z++) {
      for (int y = 0; y < info.dimensions[1]; y++) {
        auto idx = slice + y * info.dimensions[0] +
                   z * info.dimensions[0] * info.dimensions[1];
        result.data.emplace_back(buffer[idx]);
      }
    }
    break;
  }
  case Axis::Y: {
    if (slice >= info.dimensions[1]) {
      throw runtime_error("slice idx overflow");
    }
    result.width = info.dimensions[0];
    result.height = info.dimensions[2];
    result.data.reserve(result.width * result.height * sizeof(uint8_t));
    for (int z = 0; z < info.dimensions[2]; z++) {
      for (int x = 0; x < info.dimensions[0]; x++) {
        auto idx = x + slice * info.dimensions[0] +
                   z * info.dimensions[0] * info.dimensions[1];
        result.data.emplace_back(buffer[idx]);
      }
    }
    break;
  }
  case Axis::Z: {
    if (slice >= info.dimensions[2]) {
      throw runtime_error("slice idx overflow");
    }
    result.width = info.dimensions[0];
    result.height = info.dimensions[1];
    result.data.reserve(result.width * result.height * sizeof(uint8_t));
    for (int y = 0; y < info.dimensions[1]; y++) {
      for (int x = 0; x < info.dimensions[0]; x++) {
        auto idx = x + y * info.dimensions[0] +
                   slice * info.dimensions[0] * info.dimensions[1];
        result.data.emplace_back(buffer[idx]);
      }
    }
    break;
  }
  default:
    break;
  }

  return result;
}

auto Dataset::Load(const char *path) -> Dataset {
  auto ext = get_file_extension(path);
  if (ext == ".json") {
    RawReader reader(path);
    return reader.load();
  }

  if (ext == ".mrc") {
    MRCReader reader(path);
    return reader.load();
  }

  throw runtime_error(string("unsupported dataset extension: ") + ext);
}

} // namespace voxer