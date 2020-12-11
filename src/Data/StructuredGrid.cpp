#include <voxer/Data/StructuredGrid.hpp>
#include <voxer/IO/MRCReader.hpp>
#include <voxer/IO/RawReader.hpp>
#include <voxer/IO/utils.hpp>
#include <voxer/Mappers/StructuredGridHistogramMapper.hpp>

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
    for (uint32_t z = 0; z < info.dimensions[2]; z++) {
      for (uint32_t y = 0; y < info.dimensions[1]; y++) {
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
    for (uint32_t z = 0; z < info.dimensions[2]; z++) {
      for (uint32_t x = 0; x < info.dimensions[0]; x++) {
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
    for (uint32_t y = 0; y < info.dimensions[1]; y++) {
      for (uint32_t x = 0; x < info.dimensions[0]; x++) {
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

auto StructuredGrid::Load(const char *path) -> std::shared_ptr<StructuredGrid> {
  auto ext = get_file_extension(path);
  if (ext == ".raw") {
    RawReader reader(path);
    return reader.load();
  }

  if (ext == ".mrc" || ext == ".st") {
    MRCReader reader(path);
    return reader.load();
  }

  throw runtime_error(string("unsupported dataset extension: ") + ext);
}

auto StructuredGrid::get_histogram() const -> std::vector<uint32_t> {
  StructuredGridHistogramMapper mapper{};
  return mapper.map(*this);
}

StructuredGrid::Axis StructuredGrid::get_axis(const char *str) {
  return static_cast<Axis>(*str - 'x');
}

StructuredGrid StructuredGrid::operator-(const StructuredGrid &rhs) const {
  auto &lhs = *this;

  StructuredGrid dataset{};
  uint32_t limit = 0;

  if (lhs.buffer.size() > rhs.buffer.size()) {
    limit = rhs.buffer.size();
    dataset.info = rhs.info;
  } else {
    dataset.info = lhs.info;
    limit = lhs.buffer.size();
  }
  dataset.buffer.resize(limit);

  for (size_t i = 0; i < limit; i++) {
    dataset.buffer[i] = (lhs.buffer[i] - rhs.buffer[i] + 255) / 2;
  }

  return dataset;
}

} // namespace voxer