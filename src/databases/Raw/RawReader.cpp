#include "databases/Raw/RawReader.hpp"
#include "databases/conversion.hpp"
#include "voxer/utils.hpp"
#include <cassert>
#include <iostream>
#include <iterator>
#include <memory>
#include <rapidjson/document.h>
#include <sstream>

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

RawReader::RawReader(const char *json_path) {
  ifstream json;
  json.open(json_path);
  if (!json.is_open()) {
    throw runtime_error(string("cannot open file: ") + json_path);
  }

  stringstream buffer;
  buffer << json.rdbuf();
  json.close();

  rapidjson::Document document;
  document.Parse(buffer.str().c_str(), buffer.str().size());

  if (!document.IsObject()) {
    throw JSON_error("root", "object");
  }

  auto desc = document.GetObject();

  auto it = desc.FindMember("path");
  if (it == desc.end() || !(it->value.IsString())) {
    throw JSON_error("root.path", "string");
  }

  fs.open(it->value.GetString(), ios::binary);
  fs.unsetf(ios::skipws);
  if (!fs.is_open()) {
    throw runtime_error(string("cannot load file: ") + it->value.GetString());
  }

  it = desc.FindMember("type");
  if (it == desc.end() || !(it->value.IsString())) {
    throw JSON_error("root.type", "string");
  }

  it = desc.FindMember("dimensions");
  if (it == desc.end() || !(it->value.IsArray()) ||
      it->value.GetArray().Size() != 3) {
    throw JSON_error("root.dimensions", "array of 3 integers");
  }

  auto dimension_json = it->value.GetArray();
  for (int i = 0; i < 3; i++) {
    dimensions[i] = dimension_json[i].GetInt();
  }

  string type = it->value.GetString();
  if (type == "uint8") {
    value_type = ValueType::UINT8;
  } else if (type == "float") {
    value_type = ValueType::FLOAT;
  } else {
    throw runtime_error(string("unsupported value type: ") + type);
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

auto RawReader::load_region(__attribute__((unused))
                            const std::array<uint16_t, 3> &begin,
                            __attribute__((unused))
                            const std::array<uint16_t, 3> &end) -> Dataset {
  // TODO: load subregion
  throw runtime_error("not support loading subregion");
}

} // namespace voxer
