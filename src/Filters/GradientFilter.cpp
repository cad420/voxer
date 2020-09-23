#include <cmath>
#include <voxer/Filters/GradientFilter.hpp>

using namespace std;

namespace {

float norm(float x, float y, float z) { return sqrtf(x * x + y * y + z * z); }

float intpValue(float v0, float v1, float v2) {
  return (1.0f * v0 + 4.0f * v1 + 1.0f * v2) / 6.0f;
}

float intpGrad(float v0, float v1, float v2) {
  return (-1.0f * v0 + 0.0f * v1 + 1.0f * v2) / 2.0f;
};

} // namespace

namespace voxer {

vector<uint8_t> GradientFilter::process(StructuredGrid &data) {
  auto &info = data.info;
  auto &buffer = data.buffer;
  auto &dimension = info.dimensions;

  auto getValue = [&buffer, &dimension](int x, int y, int z) {
    if (x < 0 || y < 0 || z < 0 || x >= dimension[0] || y >= dimension[1] ||
        z >= dimension[2])
      return static_cast<uint8_t>(0);
    int index = z * dimension[2] * dimension[1] + y * dimension[0] + x;
    return buffer[index];
  };

  auto calGradient = [this, buffer, data, getValue](int px, int py, int pz) {
    uint8_t t0[3][3][3];
    float t1[3][3];
    float t2[3];
    array<float, 3> direction;
    for (int i = -1; i < 2; i++) {     // z
      for (int j = -1; j < 2; j++) {   // y
        for (int k = -1; k < 2; k++) { // x
          auto res = 0;
          t0[i + 1][j + 1][k + 1] = getValue(px + k, py + j, pz + i);
        }
      }
    }
    int x, y, z;
    // for x-direction
    for (z = 0; z < 3; z++) {
      for (y = 0; y < 3; y++) {
        t1[z][y] = intpGrad(t0[z][y][0], t0[z][y][1], t0[z][y][2]);
      }
    }
    for (z = 0; z < 3; z++) {
      t2[z] = intpValue(t1[z][0], t1[z][1], t1[z][2]);
    }
    direction[0] = intpValue(t2[0], t2[1], t2[2]);
    // for y-direction
    for (z = 0; z < 3; z++) {
      for (x = 0; x < 3; x++) {
        t1[z][x] = intpGrad(t0[z][0][x], t0[z][1][x], t0[z][2][x]);
      }
    }
    for (int z = 0; z < 3; z++) {
      t2[z] = intpValue(t1[z][0], t1[z][1], t1[z][2]);
    }
    direction[1] = intpValue(t2[0], t2[1], t2[2]);
    // for z-direction
    for (y = 0; y < 3; y++) {
      for (x = 0; x < 3; x++) {
        t1[y][x] = intpGrad(t0[0][y][x], t0[1][y][x], t0[2][y][x]);
      }
    }
    for (y = 0; y < 3; y++) {
      t2[y] = intpValue(t1[y][0], t1[y][1], t1[y][2]);
    }
    direction[2] = intpValue(t2[0], t2[1], t2[2]);

    return direction;
  };

  vector<uint8_t> result;
  result.resize(dimension[0] * dimension[1] * dimension[2], 0.0f);
  //#pragma omp parallel for
  auto offset_z = dimension[1] * dimension[0];
  for (size_t z = 0; z < dimension[2]; z++) {
    for (size_t y = 0; y < dimension[1]; y++) {
      for (size_t x = 0; x < dimension[0]; x++) {
        size_t index = z * offset_z + y * dimension[0] + x;
        result[4 * index + 3] = buffer[index];
        auto gradient = calGradient(x, y, z);
        if (norm(gradient[0], gradient[1], gradient[2]) > 1e-3) {
//          gradient = normalize(gradient);
          result[4 * index + 0] =
              static_cast<uint8_t>((gradient[0] + 1.0) / 2.0 * 255 + 0.5);
          result[4 * index + 1] =
              static_cast<uint8_t>((gradient[1] + 1.0) / 2.0 * 255 + 0.5);
          result[4 * index + 2] =
              static_cast<uint8_t>((gradient[2] + 1.0) / 2.0 * 255 + 0.5);
        } else {
          result[4 * index + 0] = 128;
          result[4 * index + 1] = 128;
          result[4 * index + 2] = 128;
        }
      }
    }
  }

  return result;
}

} // namespace voxer