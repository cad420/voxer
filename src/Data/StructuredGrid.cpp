#include "IO/MRC/MRCReader.hpp"
#include "IO/Raw/RawReader.hpp"
#include <voxer/Data/StructuredGrid.hpp>
#include <voxer/utils.hpp>

using namespace std;

namespace voxer {

auto StructuredGrid::get_slice(Axis axis, uint32_t slice) const -> Image {
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

auto StructuredGrid::Load(const char *path) -> StructuredGrid {
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