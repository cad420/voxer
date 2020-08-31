#include "Store/DatasetStore.hpp"
#include "DataModel/DatasetCollection.hpp"
#include "utils.hpp"
#include <fmt/core.h>
#include <seria/deserialize.hpp>
#include <stdexcept>
#include <string>
#include <voxer/Data/StructuredGrid.hpp>
#include <voxer/Filters/differ.hpp>
#include <voxer/IO/MRCReader.hpp>
#include <voxer/IO/RawReader.hpp>
#include <voxer/IO/utils.hpp>

using namespace std;

namespace voxer::remote {

vector<shared_ptr<StructuredGrid>>
DatasetStore::load_from_file(const string &filepath) {
  ifstream fs(filepath);
  if (!fs.good() || !fs.is_open()) {
    throw runtime_error("cannot open file: " + filepath);
  }
  stringstream sstr;
  sstr << fs.rdbuf();
  auto json = sstr.str();
  return this->load_from_json(json.c_str(), json.size());
}

vector<shared_ptr<StructuredGrid>>
DatasetStore::load_from_json(const char *text, uint32_t size) {
  m_document.Parse(text, size);
  vector<shared_ptr<StructuredGrid>> datasets;
  if (m_document.IsArray()) {
    const auto &items = m_document.GetArray();
    for (auto &item : items) {
      datasets.emplace_back(load_one(item));
    }
  } else {
    datasets.emplace_back(load_one(m_document.GetObject()));
  }

  fmt::print("load {} datasets.\n", datasets.size());

  return datasets;
}

shared_ptr<StructuredGrid>
DatasetStore::load_one(const rapidjson::Value &json) {
  if (!json.IsObject()) {
    throw JSON_error("dataset", "object");
  }

  Dataset dataset_desc{};
  seria::deserialize(dataset_desc, json);

  auto it = m_datasets.find(dataset_desc.id);
  if (it != m_datasets.end()) {
    return it->second;
  }

  shared_ptr<StructuredGrid> dataset{};
  auto ext = get_file_extension(dataset_desc.path);
  if (ext == ".raw") {
    RawReader reader(dataset_desc.path.c_str());
    dataset = reader.load();
  } else if (ext == ".mrc") {
    MRCReader reader(dataset_desc.path);
    dataset = reader.load();
  } else {
    throw runtime_error("unknown dataset format: " + ext);
  }
  m_datasets.emplace(dataset_desc.id, dataset);

  return dataset;

  /*DatasetCollection collection {};
  seria::deserialize(collection, json);
  for (const auto &variable : collection.variables) {
    auto idx = string::npos;
    if (variable.timesteps > 1) {
      idx = variable.path.find("[step]");
      if (idx == string::npos) {
        throw runtime_error(
            "dataset path should contain [step] when timesteps > 1");
      }
    }

    for (size_t i = 0; i < variable.timesteps; i++) {
      auto real_path = variable.path;
      if (idx != std::string::npos) {
        // substitue [step]
        real_path.replace(idx, idx + 6, to_string(i));
      }

      shared_ptr<StructuredGrid> dataset{};
      auto ext = get_file_extension(real_path);
      if (ext == ".raw") {
        if (collection.dimensions[0] == 0 || collection.type.empty()) {
          throw runtime_error("dimensions and value type information is "
                              "needed for raw dataset.");
        }
        RawReader reader(real_path, collection.dimensions,
                         collection.type == "float" ? ValueType::FLOAT
                                                    : ValueType::UINT8);
        dataset = reader.load();
      } else if (ext == ".mrc") {
        MRCReader reader(real_path);
        dataset = reader.load();
      } else {
        throw runtime_error("unknown dataset format: " + ext);
      }
      Dataset desc{};
      desc.name = collection.name;
      desc.variable = variable.name;
      desc.timestep = i;
      // desc.histogram = calculate_histogram(dataset);
      m_datasets.emplace(desc, move(dataset));
    }
  }*/
}

auto DatasetStore::get(const voxer::remote::Dataset &desc) const
    -> const shared_ptr<StructuredGrid> & {
  return get(desc.id);
}

auto DatasetStore::get(uint32_t id) const
-> const shared_ptr<StructuredGrid> & {
  const auto it = m_datasets.find(id);
  if (it == m_datasets.end()) {
    throw runtime_error("cannot find dataset");
  }

  return it->second;
}

//
// auto DatasetStore::get_or_create(const SceneDataset &scene_dataset,
//                                 const vector<SceneDataset> &scene_datasets)
//    -> const voxer::Dataset & {
//  if (!scene_dataset.diff) {
//    return this->get(scene_dataset.name, scene_dataset.variable,
//                     scene_dataset.timestep);
//  }
//
//  // TODO: ensure scene_dataset.parent < (current idx in scene_datasets), and
//  // all previous datasets are processed
//  auto &parent = scene_datasets[scene_dataset.parent];
//  // no recursion
//  auto &parent_dataset =
//      this->get(parent.name, parent.variable, parent.timestep);
//
//  // TODO: ensure scene_dataset.another < (current idx in scene_datasets), and
//  // all previous datasets are processed
//  auto &another = scene_datasets[scene_dataset.another];
//  auto &another_dataset =
//      this->get(another.name, another.variable, another.timestep);
//
//  auto id = parent_dataset.id + "-" + another_dataset.id;
//  if (temp_datasets.find(id) != temp_datasets.end()) {
//    return temp_datasets.at(id);
//  }
//
//  auto differed_buffer = differ(parent_dataset.buffer,
//  another_dataset.buffer); Dataset dataset{move(id),
//                  parent_dataset.name + "-" + another.name + "-differed",
//                  "default",
//                  0,
//                  parent_dataset.info,
//                  move(differed_buffer)};
//  temp_datasets.emplace(id, dataset);
//  return temp_datasets.at(id);
//}
//
// auto DatasetStore::print() const -> string {
//  if (m_datasets.empty()) {
//    return "[]";
//  }
//
//  string res = "[";
//  for (auto &item : m_datasets) {
//    res += "{";
//    auto &name = item.first;
//    res += fmt::format(R"("name":"{}","variables":[)", name);
//    auto &variable_lookup_table = item.second;
//    if (variable_lookup_table.empty()) {
//      res += "]},";
//      continue;
//    }
//
//    for (auto &variable : variable_lookup_table) {
//      auto &variable_name = variable.first;
//      auto &timestep_lookup_table = variable.second;
//      res += fmt::format(R"({{"name":"{}","timesteps":{}}},)", variable_name,
//                         timestep_lookup_table.size());
//    }
//    res[res.find_last_of(',')] = ']';
//    res += "},";
//  }
//  res[res.find_last_of(',')] = ']';
//  return res;
//}

void DatasetStore::add_from_json(const char *text, uint32_t size) {
  rapidjson::Document current{};
  current.Parse(text, size);
  if (current.HasParseError()) {
    throw runtime_error(string("Parsing JSON error, code: ") +
                        to_string(current.GetParseError()));
  } else if (!current.IsObject()) {
    throw runtime_error("dataset should be an object");
  }
  this->load_one(current);
}

} // namespace voxer::remote