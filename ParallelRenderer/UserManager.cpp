#include "UserManager.h"
#include "ParallelRenderer/DatasetManager.h"
#include "ospray/ospray_cpp/Volume.h"

using namespace std;
using namespace ospray::cpp;
using namespace ospcommon;

extern DatasetManager datasets;

UserManager::UserManager() {
  const vector<string> USERS = {"tester"};
  for (auto id : USERS) {
    this->users.emplace(id, User(id));
  }
}

User &UserManager::get(string id) {
  auto search = this->users.find(id);
  if (search == this->users.end()) {
    throw string("User ") + id + " not found";
  }
  return search->second;
}

void User::load(string id) {
  auto &dataset = datasets.get(id);

  Volume volume("shared_structured_volume");
  volume.set("voxelType", dataset.dtype.c_str());
  volume.set("dimensions", dataset.dimensions);
  Data data(dataset.buffer.size(), OSP_UCHAR, dataset.buffer.data(),
            OSP_DATA_SHARED_BUFFER);
  data.commit();
  volume.set("voxelData", data);
  volume.set("gridOrigin", vec3f(-dataset.dimensions / 2));

  this->volumes.emplace(id, volume);
}

Volume &User::get(string volume) {
  auto search = this->volumes.find(volume);
  if (search == this->volumes.end()) {
    throw string("Volume ") + volume + " not found";
  }
  return search->second;
}