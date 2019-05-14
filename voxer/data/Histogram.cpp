#include "voxer/data/Histogram.hpp"
#include "voxer/utils/Debugger.hpp"

using namespace std;

static Debugger debug("data::Histogram");

vector<unsigned int> createHistogram(vector<unsigned char> &data) {
  vector<unsigned int> result(256, 0);
  for (auto value : data) {
    result[value]++;
  }
  return result;
}