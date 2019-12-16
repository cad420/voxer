#include "voxer/filter/differ.hpp"

using namespace std;

namespace voxer {

auto differ(const Dataset &lhs, const Dataset &rhs) -> Dataset {
  auto &meta = lhs.buffer.size() > rhs.buffer.size() ? lhs.meta : rhs.meta;
  auto limit = lhs.buffer.size() > rhs.buffer.size() ? rhs.buffer.size()
                                                     : lhs.buffer.size();
  Dataset dataset{meta};
  dataset.buffer.resize(limit);
  for (auto i = 0; i < limit; i++) {
    dataset.buffer[i] = (lhs.buffer[i] - rhs.buffer[i] + 255) / 2;
  }
  return dataset;
}

} // namespace voxer
