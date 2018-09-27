#include "Histogram.h"
#include "../util/Debugger.h"

using namespace std;

static Debugger debug("data::Histogram");

vector<unsigned int> createHistogram(vector<unsigned char> &data) {
  vector<unsigned int> result(255, 0);
  for (auto value : data) {
    result[value]++;
  }
  return result;
}