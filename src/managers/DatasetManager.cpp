#include <voxer/DatasetManager.hpp>
#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/istreamwrapper.h"
#include "data/Histogram.hpp"
#include <ospray/ospcommon/vec.h>
#include <utility>

using namespace std;
using vec3sz = ospcommon::vec_t<size_t, 3>;

namespace voxer {

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
    string variable =
        info.HasMember("variable") ? info["variable"].GetString() : "";
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
      if (variable.size() > 0) {
        _name += "-" + variable;
      }
      if (timesteps > 1) {
        auto str = to_string(step);
        if (str.size() == 1) {
          str = "0" + str;
        }
        _filepath.replace(_filepath.end() - 10, _filepath.end() - 4, str);
        _name += "-" + str;
      }
      Dataset dataset(ospcommon::vec3i(dimensions), dtype);
      auto total = dimensions.x * dimensions.y * dimensions.z;
      auto file = fopen(_filepath.c_str(), "rb");
      int voxelSize = sizeof(unsigned char);
      size_t read = fread(dataset.buffer.data(), sizeForType, total, file);
      if (read != total) {
        throw "Read volume data " + _name + " failed\n";
      }
      dataset.histogram = createHistogram(dataset.buffer);
      datasets.emplace(_name, dataset);
    }
  }
}

Dataset &DatasetManager::get(const string name) {
  auto search = datasets.find(name);
  if (search == datasets.end()) {
    throw "Volume " + name + " not found";
  }
  return search->second;
}

void DatasetManager::add(string name, Dataset &&exist) {
  datasets.emplace(name, exist);
}

}
