#include "VolumeConfig.h"
#include "../DatasetManager.h"
#include "../UserManager.h"
#include "../data/Differ.h"
#include "../data/Mask.h"
#include <vector>

using namespace std;
using namespace ospcommon;

extern DatasetManager datasets;
extern UserManager users; 

string nameOfDataset(rapidjson::Value &params) {
  if (!params.HasMember("name") || !params["name"].IsString()) {
    throw "raw dataset config should have propery `name` of type string";
  }
  return params["name"].GetString();
}

VolumeConfig::VolumeConfig(rapidjson::Value &params) {
  if (!params.HasMember("id") || !params["id"].IsString()) {
    throw "volume config should have propery `id` with type string!";
  }
  this->id = params["id"].GetString();

  if (!params.HasMember("tfcn")) {
    throw "volume config should have propery `tfcn`";
  }
  auto &tfcnParams = params["tfcn"];
  this->tfcnConfig = TransferFunctionConfig(tfcnParams);

  this->translate = vec3f(0, 0, 0);
  this->scale = 1.0;

  if (params.HasMember("ranges")) {
    auto &ranges = params["ranges"];
    for (auto &range : ranges.GetArray()) {
      if (range.HasMember("start") && range.HasMember("end")) {
        this->ranges.push_back(Range{
          start : range["start"].GetInt(),
          end : range["end"].GetInt(),
        });
      }
    }
  }

  if (!params.HasMember("dataset") || !params["dataset"].IsObject()) {
    throw "volume config should have propery `dataset` of type dataset";
  }
  auto &datasetParams = params["dataset"];
  // DFS, handle dataset
  vector<rapidjson::Value *> stack;
  stack.push_back(&datasetParams);
  DatasetConfig dataset;
  bool end = false;
  while (!stack.empty()) {
    auto current = stack.back();
    auto &target = *current;
    if (!target.HasMember("type") || !target["type"].IsString()) {
      throw "Dataset config should have propery `type` of type string!";
    }

    string type(target["type"].GetString());
    if (type == "dataset") {
      end = true;
      dataset.name = nameOfDataset(target);
      const auto &d = datasets.get(dataset.name);
      dataset.dimensions = d.dimensions;
    } else if (type == "differ") {
      end = true;
      if (!target.HasMember("first") || !target["first"].IsObject()) {
        throw "differ dataset config should have propery `first` of type raw "
              "dataset";
      }
      if (!target.HasMember("second") || !target["second"].IsObject()) {
        throw "differ dataset config should have propery `second` of type raw "
              "dataset";
      }
      string firstDatasetName = nameOfDataset(target["first"]);
      string secondDatasetName = nameOfDataset(target["second"]);
      string nameOfDifferedDataset = firstDatasetName + "-" + secondDatasetName;
      if (!datasets.has(nameOfDifferedDataset)) {
        createDatasetByDiff(firstDatasetName, secondDatasetName);
        auto &user = users.get("tester");
        user.load(nameOfDifferedDataset);
      }
      dataset.name = nameOfDifferedDataset;
    } else if (type == "clipping") {
      if (!end) {
        if (!target.HasMember("dataset") || !target["dataset"].IsObject()) {
          throw "clipping dataset config should have propery `dataset` of type "
                "dataset";
        }
        stack.push_back(&target["dataset"]);
        continue;
      }

      if (!target.HasMember("lower") || !target["lower"].IsArray()) {
        throw "differ dataset config should have propery `lower` of type array";
      }
      auto &lowerParams = target["lower"];
      vec3f lower{lowerParams[0].GetFloat(), lowerParams[1].GetFloat(),
                  lowerParams[2].GetFloat()};
      dataset.clipingBoxLower += lower;

      if (!target.HasMember("upper") || !target["upper"].IsArray()) {
        throw "differ dataset config should have propery `upper` of type array";
      }
      auto &upperParams = target["upper"];
      vec3f upper{upperParams[0].GetFloat(), upperParams[1].GetFloat(),
                  upperParams[2].GetFloat()};
      dataset.clipingBoxUpper += upper;
    } else if (type == "transformation") {
      if (!end) {
        if (!target.HasMember("dataset") || !target["dataset"].IsObject()) {
          throw "transformation dataset config should have propery `dataset` of type "
                "dataset";
        }
        stack.push_back(&target["dataset"]);
        continue;
      }
      if (target.HasMember("x") || target.HasMember("y") ||
          target.HasMember("z")) {
        vec3f translate{target["x"].GetFloat(), target["y"].GetFloat(),
                        target["z"].GetFloat()};
        this->translate += translate;
      }
      if (target.HasMember("scale")) {
        auto scale = target["scale"].GetFloat();
        this->scale *= scale;
      }
    } else if (type == "scatter") {
      end = true;
      if (!target.HasMember("first") || !target["first"].IsObject()) {
        throw "differ dataset config should have propery `first` of type raw "
              "dataset";
      }
      if (!target.HasMember("second") || !target["second"].IsObject()) {
        throw "differ dataset config should have propery `second` of type raw "
              "dataset";
      }
      string firstDatasetName = nameOfDataset(target["first"]);
      string secondDatasetName = nameOfDataset(target["second"]);
      vec2f rangeFirst, rangeSecond;

      string nameOfDifferedDataset = firstDatasetName + "-" + secondDatasetName;
      if (!datasets.has(nameOfDifferedDataset)) {
        createDatasetByMask(firstDatasetName, secondDatasetName, rangeFirst,
                            rangeSecond);
      }
      dataset.name = nameOfDifferedDataset;
    } else {
      throw "Unsupported dataset";
    }
    stack.pop_back();
  }
  this->datasetConfig = dataset;
}