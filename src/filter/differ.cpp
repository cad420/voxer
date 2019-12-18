#include "voxer/filter/differ.hpp"

using namespace std;

namespace voxer {

auto differ(const std::vector<uint8_t> &lhs, const std::vector<uint8_t> &rhs)
    -> std::vector<uint8_t> {
  auto limit = lhs.size() > rhs.size() ? rhs.size() : lhs.size();
  vector<uint8_t> buffer(limit);
  for (auto i = 0; i < limit; i++) {
    buffer[i] = (lhs[i] - rhs[i] + 255) / 2;
  }
  return buffer;
}

} // namespace voxer
