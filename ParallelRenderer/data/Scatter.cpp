#include "Scatter.h"
#include <iostream>

using namespace std;
using namespace ospcommon;

Scatter createScatter(vector<unsigned char> &A, vector<unsigned char> &B,
                      vec3i &dimensions) {
  Scatter scatter;

  const int stepX = 2;
  const int stepY = 2;
  const int stepZ = 2;
  for (auto i = 0; i < 256; i++)
    for (auto j = 0; j < 256; j++)
      scatter.points[i][j] = 0;

  const auto tmp = dimensions.x * dimensions.y;
  scatter.max = 0;

  for (auto z = 0; z < dimensions.z; z += stepZ) {
    for (auto y = 0; y < dimensions.y; y += stepY) {
      for (auto x = 0; x < dimensions.x; x += stepX) {
        const auto offset = x + y * dimensions.x + z * tmp;
        auto &va = A[offset];
        auto &vb = B[offset];
        scatter.points[va][vb] += 1;
        if (scatter.points[va][vb] > scatter.max) {
          scatter.max = scatter.points[va][vb];
        }
      }
    }
  }

  return scatter;
}