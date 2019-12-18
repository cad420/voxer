#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <simdjson/jsonparser.h>
#include <string>
#include <vector>

namespace voxer {

struct Dataset;
enum class DatasetValueType { FLOAT, UCHAR };

struct FieldInfo {
  std::array<uint32_t, 3> dimensions = {0, 0, 0};
  DatasetValueType type = {DatasetValueType::UCHAR};
  uint8_t type_size = sizeof(uint8_t);
};

struct Dataset {
  std::string id = "";
  FieldInfo meta = {};
  std::shared_ptr<std::vector<uint8_t>> buffer = nullptr;
  std::vector<uint32_t> histogram = {};
};

} // namespace voxer
