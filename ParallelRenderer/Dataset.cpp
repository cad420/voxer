#include "Dataset.h"
#include "ospray/ospcommon/vec.h"
#include "third_party/RawReader/RawReader.h"
#include <string>

using namespace std;
using namespace ospcommon;
using vec3sz = vec_t<size_t, 3>;

Dataset::Dataset(string name, vec3i _dimensions, size_t dtypeSize)
    : name(name), dimensions(_dimensions) {
  voxelSize = dtypeSize * dimensions.x * dimensions.y * dimensions.z;
  data = vector<unsigned char>(voxelSize);
}

DatasetManager::DatasetManager() : datasets() {
  string file = "/d/data/csafe-heptane-302-volume/csafe-heptane-302-volume.raw";
  int dtypeSize = 8;
  vec3i dimensions = {302, 302, 302};
  gensv::RawReader reader(file, vec3sz(dimensions), dtypeSize);
  Dataset dataset("heptane", dimensions, dtypeSize);
  // reader.readRegion(brickId * brickDims - vec3sz(ghostOffset),
  //                     vec3sz(dimensions), dataset.data.data());
}

DatasetManager::DatasetManager() {}