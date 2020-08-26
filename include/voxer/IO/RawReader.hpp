#pragma once
#include <array>
#include <fstream>
#include <voxer/Data/StructuredGrid.hpp>
#include <voxer/IO/AbstractDatasetReader.hpp>

namespace voxer {

struct RawReader : public AbstractDatasetReader {
  std::ifstream fs;
  std::array<uint32_t, 3> dimensions;
  ValueType value_type;

  explicit RawReader(const char *filepath);
  RawReader(const std::string &filepath,
            const std::array<uint32_t, 3> &dimensions, ValueType value_type);

  auto load() -> std::unique_ptr<StructuredGrid> override;
  auto load_region(const std::array<uint16_t, 3> &begin,
                   const std::array<uint16_t, 3> &end)
      -> std::unique_ptr<StructuredGrid>;
};

} // namespace voxer