#include "Slice.h"
#include "Encoder.h"
#include <ospray/ospray_cpp.h>
#include <utility>

using namespace std;
using namespace ospcommon;

vector<unsigned char> value(Dataset dataset, uint32_t startX, uint32_t endX,
                            uint32_t startY, uint32_t endY, uint32_t startZ,
                            uint32_t endZ) {
  auto &dimensions = dataset.dimensions;

  auto xoy = dimensions.x * dimensions.y;
  vector<unsigned char> result;
  result.reserve((endX - startX) * (endY - startY) * (endZ - startZ) * 3);
  for (uint32_t z = startZ; z < endZ; z++) {
    for (uint32_t y = startY; y < endY; y++) {
      for (uint32_t x = startX; x < endX; x++) {
        auto offset = x + y * dimensions.x + z * xoy;
        result.push_back(dataset.buffer[offset]);
        result.push_back(dataset.buffer[offset]);
        result.push_back(dataset.buffer[offset]);
      }
    }
  }

  return result;
}

Image createSlice(const Dataset &dataset, const Axis axis, int index) {
  auto &dimensions = dataset.dimensions;
  auto startX = 0, startY = 0, startZ = 0, endX = dimensions.x,
       endY = dimensions.y, endZ = dimensions.z;
  vec2ui size(dimensions.y, dimensions.z);
  if (axis == Axis::x) {
    startX = index;
    endX = index + 1;
  } else if (axis == Axis::y) {
    startY = index;
    endY = index + 1;
    size[0] = dimensions.x;
    size[1] = dimensions.z;
  } else {
    startZ = index;
    endZ = index + 1;
    size[0] = dimensions.x;
    size[1] = dimensions.y;
  }

  auto slice = value(dataset, startX, endX, startY, endY, startZ, endZ);

  Encoder encoder;
  auto image = encoder.encode(slice, size, "JPEG", false);
            cout << "after encode" << endl;

  return image;
}
