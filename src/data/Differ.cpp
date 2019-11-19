#include "data/Differ.hpp"
#include "data/Histogram.hpp"
#include "voxer/DatasetManager.hpp"

using namespace std;

namespace voxer {

extern DatasetManager datasets;

string createDatasetByDiff(string first, string second) {
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
    dataset.buffer[i] =
        (firstDataset.buffer[i] - secondDataset.buffer[i] + 255) / 2;
  }
  auto name = first + "-" + second;
  dataset.histogram = createHistogram(dataset.buffer);
  datasets.add(name, move(dataset));
  return name;
}

}
