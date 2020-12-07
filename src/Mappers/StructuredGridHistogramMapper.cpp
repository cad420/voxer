#include <voxer/Mappers/StructuredGridHistogramMapper.hpp>

using namespace std;

namespace voxer {

std::vector<uint32_t>
StructuredGridHistogramMapper::map(const StructuredGrid &dataset) const {
  vector<uint32_t> result(bins, 0);

  for (auto value : dataset.buffer) {
    if (value == 0 || value == 255)
      continue;
    result[value]++;
  }

  return result;
}

} // namespace voxer
