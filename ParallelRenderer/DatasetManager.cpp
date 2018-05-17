#include "DatasetManager.h"
#include "ospray/ospcommon/vec.h"
#include "third_party/RawReader/RawReader.h"
#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/istreamwrapper.h"
#include <utility>

using namespace std;
using vec3sz = ospcommon::vec_t<size_t, 3>;

void DatasetManager::load(string filepath) {
  rapidjson::Document d;
  ifstream filestream;
  filestream.open(filepath);
  if (!filestream.is_open()) {
    cerr << "Unable to open file datasets.json!" << endl;
    exit(1);
  }
  rapidjson::IStreamWrapper isw(filestream);
  d.ParseStream(isw);
  if (!d.IsArray()) {
    cerr << "Invalid data file!" << endl;
    exit(1);
  }

  vec3sz dimensions;
  for (auto &info : d.GetArray()) {
    if (!info.IsObject()) {
      cerr << "Invalid data file!" << endl;
      exit(1);
    }
    auto name = string(info["name"].GetString());
    auto filepath = string(info["path"].GetString());
    auto dimsData = info["dimensions"].GetArray();
    for (auto dim = dimsData.Begin(); dim != dimsData.end(); dim++) {
      dimensions[dim - dimsData.Begin()] = dim->GetInt();
    }
    auto dtype = string(info["dtype"].GetString());
    int sizeForType = 0;
    if (dtype == "uchar" || dtype == "char") {
      sizeForType = 1;
    } else if (dtype == "float") {
      sizeForType = 4;
    } else if (dtype == "double") {
      sizeForType = 8;
    }
    gensv::RawReader reader(ospcommon::FileName(filepath), dimensions,
                            sizeForType);
    const auto upper = ospcommon::vec3f(dimensions);
    const auto halfLength = ospcommon::vec3i(dimensions) / 2;
    auto volume = gensv::loadVolume(reader, ospcommon::vec3i(dimensions), dtype);
    volume.bounds.lower -= ospcommon::vec3f(halfLength);
    volume.bounds.upper -= ospcommon::vec3f(halfLength);
    volume.volume.set("gridOrigin",
                      volume.ghostGridOrigin - ospcommon::vec3f(halfLength));
    volume.volume.commit();
    datasets[name] = volume;
  }
}

gensv::LoadedVolume& DatasetManager::get(const char *name) {
  auto search = datasets.find(name);
  if (search == datasets.end()) {
    throw string("Volume ") + name + " not found";
  }
  return search->second;
}