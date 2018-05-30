#include "GenerateSciVis.h"
#include "ospray/ospray_cpp/Data.h"
#include "ospray/ospray_cpp/TransferFunction.h"
#include "ospray/ospray_cpp/Volume.h"
#include "third_party/RawReader/RawReader.h"
#include "ospray/ospray.h"
#include <array>
#include <ospray/mpiCommon/MPICommon.h>
#include <random>

namespace gensv {

bool computeDivisor(int x, int &divisor) {
  int upperBound = std::sqrt(x);
  for (int i = 2; i <= upperBound; ++i) {
    if (x % i == 0) {
      divisor = i;
      return true;
    }
  }
  return false;
}

// Compute an X x Y x Z grid to have num bricks,
// only gives a nice grid for numbers with even factors since
// we don't search for factors of the number, we just try dividing by two
vec3i computeGrid(int num) {
  vec3i grid(1);
  int axis = 0;
  int divisor = 0;
  while (computeDivisor(num, divisor)) {
    grid[axis] *= divisor;
    num /= divisor;
    axis = (axis + 1) % 3;
  }
  if (num != 1) {
    grid[axis] *= num;
  }
  return grid;
}

LoadedVolume::LoadedVolume() : volume(nullptr), tfcn("piecewise_linear") {}

enum GhostFace {
  NEITHER_FACE = 0,
  POS_FACE = 1,
  NEG_FACE = 1 << 1,
};

/* Compute which faces of this brick we need to specify ghost voxels for,
 * to have correct interpolation at brick boundaries. Returns mask of
 * GhostFaces for x, y, z.
 */
std::array<int, 3> computeGhostFaces(const vec3i &brickId, const vec3i &grid) {
  std::array<int, 3> faces = {{NEITHER_FACE, NEITHER_FACE, NEITHER_FACE}};
  for (size_t i = 0; i < 3; ++i) {
    if (brickId[i] < grid[i] - 1) {
      faces[i] |= POS_FACE;
    }
    if (brickId[i] > 0) {
      faces[i] |= NEG_FACE;
    }
  }
  return faces;
}

size_t sizeForDtype(const std::string &dtype) {
  if (dtype == "uchar" || dtype == "char") {
    return 1;
  }
  if (dtype == "float") {
    return 4;
  }
  if (dtype == "double") {
    return 8;
  }

  return 0;
}

OSPDataType typeForString(const std::string &dtype) {
  if (dtype == "uchar" || dtype == "char") {
    return OSP_UCHAR;
  }
  if (dtype == "float") {
    return OSP_FLOAT;
  }
  if (dtype == "double") {
    return OSP_DOUBLE;
  }

  return OSP_UCHAR;
}

void loadVolume(struct LoadedVolume* vol, ospray::cpp::Data *data, const vec3i &dimensions,
                        const std::string &dtype, size_t sizeForDtype) {
  auto numRanks = static_cast<float>(mpicommon::numGlobalRanks());
  auto myRank = mpicommon::globalRank();
  numRanks = 1;
  myRank = 0;

  const vec3sz grid = vec3sz(computeGrid(numRanks));
  const vec3sz brickDims = vec3sz(dimensions) / grid;
  const vec3sz brickId(myRank % grid.x, (myRank / grid.x) % grid.y,
                       myRank / (grid.x * grid.y));
  const vec3f gridOrigin = vec3f(brickId) * vec3f(brickDims);
  const std::array<int, 3> ghosts =
      computeGhostFaces(vec3i(brickId), vec3i(grid));
  vec3sz ghostDims(0);
  for (size_t i = 0; i < 3; ++i) {
    if (ghosts[i] & POS_FACE) {
      ghostDims[i] += 1;
    }
    if (ghosts[i] & NEG_FACE) {
      ghostDims[i] += 1;
    }
  }
  const vec3sz fullDims = brickDims + ghostDims;
  const vec3i ghostOffset(ghosts[0] & NEG_FACE ? 1 : 0,
                          ghosts[1] & NEG_FACE ? 1 : 0,
                          ghosts[2] & NEG_FACE ? 1 : 0);
  vol->ghostGridOrigin = gridOrigin - vec3f(ghostOffset);

  vol->volume = ospray::cpp::Volume("shared_structured_volume");
  // vol->volume = ospray::cpp::Volume("block_bricked_volume");
  vol->volume.set("voxelType", dtype.c_str());
  vol->volume.set("dimensions", vec3i(fullDims));
  vol->volume.set("transferFunction", vol->tfcn);

  // const size_t dtypeSize = sizeForDtype(dtype);
  // std::vector<unsigned char> volumeData(
  //     fullDims.x * fullDims.y * fullDims.z * dtypeSize, 0);
  // auto size = reader.readRegion(brickId * brickDims - vec3sz(ghostOffset), vec3sz(fullDims),
  //                   volumeData.data());

  // for shared_structured_volume
  vol->volume.set("voxelData", *data);
  
  // end

  // for block_bricked_volume
  // vol->volume.setRegion(volumeData.data(), vec3i(0), vec3i(fullDims));
  // end

  vol->bounds = box3f(gridOrigin, gridOrigin + vec3f(brickDims));
}

} // namespace gensv