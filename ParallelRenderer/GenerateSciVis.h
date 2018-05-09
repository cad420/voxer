#pragma once
#include "ospray/mpiCommon/MPICommon.h"
#include "ospray/ospcommon/FileName.h"
#include "ospray/ospray_cpp/Geometry.h"
#include "ospray/ospray_cpp/TransferFunction.h"
#include "ospray/ospray_cpp/Volume.h"
#include "third_party/RawReader/RawReader.h"

namespace gensv {

using namespace ospcommon;
// Compute an X x Y x Z grid to have num bricks,
// only gives a nice grid for numbers with even factors since
// we don't search for factors of the number, we just try dividing by two

vec3i computeGrid(int num);

struct LoadedVolume {
  ospray::cpp::Volume volume;
  ospray::cpp::TransferFunction tfcn;
  box3f bounds;
  vec3f ghostGridOrigin;
  box3f worldBounds;

  LoadedVolume();
};

/* Load this rank's volume data. The volumes are placed in
 * cells of the grid computed in 'computeGrid' based on the number
 * of ranks with each rank owning a specific cell in the gridding.
 * Returns the ghostGridOrigin of the volume which may be outside the bounding
 * box, due to the ghost voxels.
 */
LoadedVolume loadVolume(RawReader &reader, const vec3i &dimensions,
                        const std::string &dtype, const vec2f &valueRange);

} // namespace gensv
