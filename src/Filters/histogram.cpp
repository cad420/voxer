#include <voxer/Filters/histogram.hpp>

using namespace std;

namespace voxer {

auto calculate_histogram(const StructuredGrid &dataset) -> vector<uint32_t> {
  vector<unsigned int> result(256, 0);
  for (auto value : dataset.buffer) {
    if (value == 0 || value == 255)
      continue;
    result[value]++;
  }
  return result;
}

} // namespace voxer
