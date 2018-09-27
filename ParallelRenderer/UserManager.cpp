#include "UserManager.h"
#include "ParallelRenderer/DatasetManager.h"
#include "ospray/ospray_cpp/Volume.h"

using namespace std;
using namespace ospray::cpp;

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
  gensv::LoadedVolume volume;

  const auto halfLength = dataset.dimensions / 2;
  volume.bounds.lower -= ospcommon::vec3f(halfLength);
  volume.bounds.upper -= ospcommon::vec3f(halfLength);
  gensv::loadVolume(volume, dataset.buffer, dataset.dimensions, dataset.dtype,
                    dataset.sizeForDType);
  volume.volume.set("gridOrigin",
                    volume.ghostGridOrigin - ospcommon::vec3f(halfLength));
  volume.volume.commit();
  this->volumes.emplace(id, volume);
}

gensv::LoadedVolume &User::get(string volume) {
  auto search = this->volumes.find(volume);
  if (search == this->volumes.end()) {
    throw string("Volume ") + volume + " not found";
  }
  return search->second;
}