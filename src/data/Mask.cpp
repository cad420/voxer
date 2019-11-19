#include "data/Mask.hpp"
#include <voxer/DatasetManager.hpp>

using namespace std;
using namespace ospcommon;

namespace voxer {
extern DatasetManager datasets;

string createDatasetByMask(string first, string second, vec2f &rangeFirst,
                           vec2f &rangeSecond) {
  auto &firstDataset = datasets.get(first.c_str());
  auto &secondDataset = datasets.get(second.c_str());
  auto dimensions = firstDataset.buffer.size() > secondDataset.buffer.size()
                        ? firstDataset.dimensions
                        : secondDataset.dimensions;
  auto limit = firstDataset.buffer.size() > secondDataset.buffer.size()
                   ? secondDataset.buffer.size()
                   : secondDataset.buffer.size();
  const auto upper = ospcommon::vec3f(dimensions);
  const auto halfLength = ospcommon::vec3i(dimensions) / 2;
  Dataset dataset(dimensions, "uchar");
  for (auto i = 0; i < limit; i++) {
    auto firstData = firstDataset.buffer[i];
    auto secondData = secondDataset.buffer[i];
    if (firstData >= rangeFirst.x && firstData <= rangeFirst.y &&
        secondData >= rangeSecond.x && secondData <= rangeSecond.y) {
      dataset.buffer[i] = firstData;
    } else {
      dataset.buffer[i] = secondData;
    }
  }
  auto name = first + "-" + second;
  datasets.add(name, move(dataset));
  return name;
}
}
