#include "VolumeConfig.h"
#include "../DatasetManager.h"
#include "../data/Differ.h"
#include <vector>

using namespace std;
using namespace ospcommon;

extern DatasetManager datasets;

string nameOfDataset(rapidjson::Value &params) {
  string name = params["name"].GetString();
  if (params.HasMember("variable")) {
    name += "-";
    name += params["variable"].GetString();
  }
  if (params.HasMember("timestep")) {
    auto timestep = params["timestep"].GetInt();
    name += "-";
    if (timestep < 10) {
      name += "0";
    }
    name += to_string(timestep);
  }
  return name;
}

VolumeConfig::VolumeConfig(rapidjson::Value &params) {
  this->id = params["id"].GetString();
  auto &datasetParams = params["dataset"];

  auto &tfcnParams = params["tfcn"];
  this->tfcnConfig = TransferFunctionConfig(tfcnParams);

  this->translate = vec3f(0, 0, 0);

  if (params.HasMember("ranges")) {
    auto &ranges = params["ranges"];
    for (auto &range : ranges.GetArray()) {
      if (range.HasMember("start") && range.HasMember("end")) {
        this->ranges.push_back(Range{
          start: range["start"].GetInt(),
          end: range["end"].GetInt(),
        });
      }
    }
  }

  // DFS, handle dataset
  vector<rapidjson::Value *> stack;
  stack.push_back(&datasetParams);
  DatasetConfig dataset;
  bool end = false;
  while (!stack.empty()) {
    auto current = stack.back();
    string type((*current)["type"].GetString());
    if (type == "dataset") {
      end = true;
      dataset.name = nameOfDataset((*current));
    } else if (type == "differ") {
      end = true;
      string firstDatasetName = nameOfDataset((*current)["first"]);
      string secondDatasetName = nameOfDataset((*current)["second"]);
      string nameOfDifferedDataset = firstDatasetName + "-" + secondDatasetName;
      if (!datasets.has(nameOfDifferedDataset)) {
        createDatasetByDiff(firstDatasetName, secondDatasetName);
      }
      dataset.name = nameOfDifferedDataset;
    } else if (type == "clip") {
      if (!end) {
        stack.push_back(&(*current)["dataset"]);
        continue;
      }
      auto &lowerParams = (*current)["lower"];
      vec3f lower{lowerParams[0].GetFloat(), lowerParams[1].GetFloat(),
                  lowerParams[2].GetFloat()};
      dataset.clipingBoxLower += lower;

      auto &upperParams = (*current)["upper"];
      vec3f upper{upperParams[0].GetFloat(), upperParams[1].GetFloat(),
                  upperParams[2].GetFloat()};
      dataset.clipingBoxUpper += upper;
    } else if (type == "translate") {
      if (!end) {
        stack.push_back(&(*current)["dataset"]);
        continue;
      }
      auto &translateParams = (*current)["translate"];
      vec3f translate{translateParams[0].GetFloat(),
                      translateParams[1].GetFloat(),
                      translateParams[2].GetFloat()};
      this->translate += translate;
    } else {
      throw "Unsupported dataset";
    }
    stack.pop_back();
  }
  this->datasetConfig = dataset;
}