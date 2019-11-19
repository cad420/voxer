#include "data/Histogram.hpp"
#include "utils/Debugger.hpp"

using namespace std;

static Debugger debug("data::Histogram");

namespace voxer {

vector<unsigned int> createHistogram(vector<unsigned char> &data) {
  vector<unsigned int> result(256, 0);
  for (auto value : data) {
    result[value]++;
  }
  return result;
}

}
