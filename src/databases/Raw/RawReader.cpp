#include "databases/Raw/RawReader.hpp"
#include "databases/conversion.hpp"
#include "voxer/utils.hpp"
#include <cassert>
#include <iostream>
#include <iterator>
#include <memory>

using namespace std;

namespace voxer {

RawReader::RawReader(const string &filepath,
                     const std::array<uint16_t, 3> &dimensions,
                     ValueType value_type)
    : dimensions(dimensions), value_type(value_type) {
  assert(dimensions[0] > 0 && dimensions[1] > 0 && dimensions[2] > 0);
  fs.open(filepath.c_str(), ios::binary);
  fs.unsetf(ios::skipws);
  if (!fs.is_open()) {
    throw runtime_error("cannot load file: " + filepath);
  }
}

auto RawReader::load() -> Dataset {
  Dataset dataset{};
  dataset.id = nanoid(5);
  dataset.info.dimensions = dimensions;
  if (value_type == ValueType::UINT8) {
    dataset.buffer.resize(dataset.info.byte_count());
    dataset.buffer.insert(dataset.buffer.begin(), istream_iterator<uint8_t>(fs),
                          istream_iterator<uint8_t>());
  } else {
    uint64_t total = dimensions[0] * dimensions[1] * dimensions[2];
    vector<float> buffer;
    buffer.resize(total);
    buffer.insert(buffer.begin(), istream_iterator<float>(fs),
                  istream_iterator<float>{});
    dataset.buffer = conversion(buffer.data(), total);
  }
  dataset.info.value_type = ValueType::UINT8;
  return dataset;
}

auto RawReader::load_region(const std::array<uint16_t, 3> &begin,
                            const std::array<uint16_t, 3> &end) -> Dataset {
  throw runtime_error("not support loading subregion");
}

} // namespace voxer
