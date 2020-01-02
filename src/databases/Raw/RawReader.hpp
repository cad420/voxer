#include "voxer/Dataset.hpp"
#include <array>
#include <fstream>

namespace voxer {

struct RawReader {
  std::ifstream fs;
  std::array<uint16_t, 3> dimensions;
  ValueType value_type;

  RawReader(const std::string &filepath,
            const std::array<uint16_t, 3> &dimensions, ValueType value_type);
  auto load() -> Dataset;
  auto load_region(const std::array<uint16_t, 3> &begin,
                   const std::array<uint16_t, 3> &end) -> Dataset;
};

} // namespace voxer