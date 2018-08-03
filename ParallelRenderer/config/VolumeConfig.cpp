#include "VolumeConfig.h"
#include "../DatasetManager.h"
#include "../data/Differ.h"
#include <vector>

using namespace std;
using namespace ospcommon;

extern DatasetManager datasets;

string nameOfDataset(rapidjson::Value &params) {
  string name;
  if (params.HasMember("timestep")) {
    auto timestep = params["timestep"].GetInt();
    name = params["name"].GetString() + to_string(timestep);
  } else {
    name = params["name"].GetString();
  }
  return name;
}

VolumeConfig::VolumeConfig(rapidjson::Value &params) {
  this->name = params["name"].GetString();
  auto &datasetParams = params["dataset"];

  auto &tfcnParams = params["tfcn"];
  this->tfcnConfig = TransferFunctionConfig(tfcnParams);

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
      dataset.translate += translate;
    } else {
      throw "Unsupported dataset";
    }
    stack.pop_back();
  }
  this->datasetConfig = dataset;
}