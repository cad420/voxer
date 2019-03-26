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
  auto buffer = dataset.buffer;

  auto xoy = dimensions.x * dimensions.y;
  auto offset = startX + startY * dimensions.x + startZ * xoy;
  vector<unsigned char> result(endX - startX + dimensions.x * (endY - startY) +
                               xoy * (endZ - startZ));

  for (uint32_t z = startZ; z < endZ; z++) {
    for (uint32_t y = startY; y < endY; y++) {
      for (uint32_t x = startX; x < endX; x++) {
        result.push_back(dataset.buffer[offset]);
        offset += 1;
      }
      offset += dimensions.x;
    }
    offset += xoy;
  }

  return result;
}

Image createSlice(Dataset dataset, int axis, int index) {
  auto &dimensions = dataset.dimensions;
  auto startX = 0, startY = 0, startZ = 0, endX = dimensions.x, endY = dimensions.y, endZ = dimensions.z;
  if (axis == 0) {
    startX = index;
    endX = index + 1;
  } else if (axis == 1) {
    startY = index;
    endY = index + 1;
  } else {
    startZ = index;
    endZ = index + 1;
  }
  auto slice = value(dataset, startX, endX, startY, endY, startZ, endZ);

  Encoder encoder;
  auto image = encoder.encode(slice, vec2ui(256, 256), "jpeg", false, true);
  return image;
}
