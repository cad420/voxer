#include <cassert>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <voxer/IO/RawReader.hpp>
#include <voxer/IO/utils.hpp>

using namespace std;

namespace voxer {

RawReader::RawReader(const string &filepath,
                     const std::array<uint32_t, 3> &dimensions,
                     ValueType value_type)
    : dimensions(dimensions), value_type(value_type) {
  assert(dimensions[0] > 0 && dimensions[1] > 0 && dimensions[2] > 0);
  fs.open(filepath.c_str(), ios::binary);
  fs.unsetf(ios::skipws);
  if (!fs.is_open()) {
    throw runtime_error("cannot load file: " + filepath);
  }
}

RawReader::RawReader(const char *filepath) {
  const char *name_rule = "raw data should be named as `name_x_y_z_datatype`, "
                          "for example: isabel_500_500_100_uint8";
  auto filename = get_file_name(filepath);
  auto splited = split(filename, "_");
  if (splited.size() < 5) {
    throw runtime_error(name_rule);
  }

  fs.open(filepath, ios::binary);
  fs.unsetf(ios::skipws);
  if (!fs.is_open()) {
    throw runtime_error(string("cannot load file: ") + filepath);
  }

  for (int i = 0; i < 3; i++) {
    auto &dim = splited[splited.size() - 4 + i];
    dimensions[i] = strtoul(dim.c_str(), nullptr, 10);
    if (dimensions[i] == 0) {
      throw runtime_error(name_rule);
    }
  }

  auto &type = splited[splited.size() - 1];
  if (type == "uint8") {
    value_type = ValueType::UINT8;
  } else if (type == "uint16") {
    value_type = ValueType::UINT16;
  } else if (type == "int16") {
    value_type = ValueType::INT16;
  } else if (type == "float") {
    value_type = ValueType::FLOAT;
  } else {
    throw runtime_error(string("unsupported value type: ") + type);
  }
}

auto RawReader::load() -> unique_ptr<StructuredGrid> {
  auto dataset = make_unique<StructuredGrid>();
  dataset->info.dimensions = dimensions;
  if (value_type == ValueType::UINT8) {
    dataset->original_range = {0.0f, 255.0f};
    dataset->buffer.reserve(dataset->info.byte_count());
    dataset->buffer.insert(dataset->buffer.begin(),
                           istream_iterator<uint8_t>(fs),
                           istream_iterator<uint8_t>());
  } else if (value_type == ValueType::INT16) {
    uint64_t total = dimensions[0] * dimensions[1] * dimensions[2];
    vector<int16_t> buffer;
    buffer.resize(total);
    fs.read(reinterpret_cast<char *>(buffer.data()), total * sizeof(int16_t));
    dataset->buffer =
        convert_int16_to_uint8(buffer.data(), total, dataset->original_range);
  } else if (value_type == ValueType::FLOAT) {
    uint64_t total = dimensions[0] * dimensions[1] * dimensions[2];
    vector<float> buffer;
    buffer.resize(total);
    fs.read(reinterpret_cast<char *>(buffer.data()), total * sizeof(float));
    dataset->buffer =
        convert_float_to_uint8(buffer.data(), total, dataset->original_range);
  } else {
    throw runtime_error(string("unsupported value type"));
  }
  dataset->info.value_type = ValueType::UINT8;
  return dataset;
}

auto RawReader::load_region(const std::array<uint16_t, 3> &begin,
                            const std::array<uint16_t, 3> &end)
    -> std::unique_ptr<StructuredGrid> {
  // TODO: load subregion
  throw runtime_error("not support loading subregion");
}

} // namespace voxer
