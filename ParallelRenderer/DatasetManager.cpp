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
    auto timesteps =
        info.HasMember("timesteps") ? info["timesteps"].GetInt() : 1;

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
    for (auto step = 1; step <= timesteps; step++) {
      auto _filepath = filepath;
      auto _name = name;
      if (timesteps > 1) {
        _filepath.replace(_filepath.end() - 10, _filepath.end() - 4,
                          to_string(step));
        _name += to_string(step);
      }
      gensv::RawReader reader(ospcommon::FileName(_filepath), dimensions,
                              sizeForType);
      Dataset dataset(ospcommon::vec3i(dimensions), dtype);
      auto size = reader.readRegion(vec3sz(0, 0, 0),
                                    dimensions, dataset.buffer.data());
      // const auto upper = ospcommon::vec3f(dimensions);
      // const auto halfLength = ospcommon::vec3i(dimensions) / 2;
      // auto volume =
      //     gensv::loadVolume(reader, ospcommon::vec3i(dimensions), dtype);
      // volume.bounds.lower -= ospcommon::vec3f(halfLength);
      // volume.bounds.upper -= ospcommon::vec3f(halfLength);
      // volume.volume.set("gridOrigin",
      //                   volume.ghostGridOrigin - ospcommon::vec3f(halfLength));
      // volume.volume.commit();
      cout << dataset.buffer.size() << endl;
      datasets.emplace(_name, dataset);
      cout << datasets[_name].buffer.size() << endl;
    }
  }
}

Dataset &DatasetManager::get(const char *name) {
  auto search = datasets.find(name);
  if (search == datasets.end()) {
    throw string("Volume ") + name + " not found";
  }
  return search->second;
}