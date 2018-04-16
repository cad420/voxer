#include "ospray/ospcommon/vec.h"
#include "third_party/RawReader/RawReader.h"
#include <vector>

using namespace std;
using vec3sz = ospcommon::vec_t<size_t, 3>;

class Dataset {
public:
  string name;
  vec3sz dimensions;
  size_t voxelSize;
  vector<unsigned char> data;
  Dataset(string name, ospcommon::vec3i dimension, size_t dtypeSize);
  ~Dataset();
};

class DatasetManager {
public:
  DatasetManager();
  ~DatasetManager();
  Dataset getDataset(string name);

private:
  vector<Dataset> datasets;
};
