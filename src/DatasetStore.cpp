#include "voxer/DatasetStore.hpp"
#include "databases/MRC/MRCReader.hpp"
#include "databases/Raw/RawReader.hpp"
#include "voxer/filter/differ.hpp"
#include "voxer/utils.hpp"
#include <cassert>
#include <fmt/core.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace voxer {

void DatasetStore::load_from_file(const string &filepath) {
  ifstream fs(filepath);
  if (!fs.good() || !fs.is_open()) {
    throw runtime_error("cannot open file: " + filepath);
  }
  this->path = filepath;
  stringstream sstr;
  sstr << fs.rdbuf();
  auto json = sstr.str();
  this->load_from_json(json.c_str(), json.size());
}

void DatasetStore::load_from_json(const char *text, uint32_t size) {
  document.Parse(text, size);
  if (!document.IsArray()) {
    throw JSON_error("root", "array");
  }

  for (auto &item : document.GetArray()) {
    load_one(item);
  }

  fmt::print("load {} datasets.\n", datasets.size());
}

void DatasetStore::load_one(const rapidjson::Value &json) {
  if (!json.IsObject()) {
    throw JSON_error("dataset", "object");
  }

  auto params = json.GetObject();
  // key values
  auto it = params.FindMember("name");
  if (it == params.end() || !(it->value.IsString())) {
    throw JSON_error("dataset.name", "string");
  }
  string name = it->value.GetString();

  auto &variable_lookup_table = lookup_table[name];

  VolumeInfo info{};
  auto has_value_type = false;
  it = params.FindMember("type");
  if (it != params.end()) {
    if (!(it->value.IsString())) {
      throw JSON_error("dataset.type", "string");
    }
    string type = it->value.GetString();
    if (type == "float") {
      info.value_type = ValueType::FLOAT;
    }
    has_value_type = true;
  }

  auto has_dimensions = false;
  it = params.FindMember("dimensions");
  if (it != params.end()) {
    if (!(it->value.IsArray()) || (it->value.GetArray().Size() != 3)) {
      throw JSON_error("dataset.dimensions", "array of three item");
    }

    auto dimensions = it->value.GetArray();
    for (size_t i = 0; i < 3; i++) {
      if (!dimensions[i].IsNumber()) {
        throw JSON_error("dimensions[" + to_string(i) + "]", "integer");
      }
      info.dimensions[i] = dimensions[i].GetInt();
    }
    has_dimensions = true;
  }

  it = params.FindMember("variables");
  if (it == params.end() || !(it->value.IsArray())) {
    throw JSON_error("dataset.variables", "array");
  }

  // into variables []
  auto variables = it->value.GetArray();
  for (auto &item : variables) {
    if (!item.IsObject()) {
      throw JSON_error("datasets.variables[i]", "object");
    }
    auto variable = item.GetObject();
    it = variable.FindMember("name");
    if (it == variable.end() && !(it->value.IsString())) {
      throw JSON_error("datasets.variables[i].name", "string");
    }
    string variable_name = it->value.GetString();
    auto &timestep_lookup_table = variable_lookup_table[variable_name];

    it = variable.FindMember("timesteps");
    if (it == variable.end() && !(it->value.IsInt())) {
      throw JSON_error("dataset.variables[i].timesteps", "integer");
    }
    const auto timesteps = static_cast<uint32_t>(it->value.GetInt());
    assert(timesteps >= 1);

    timestep_lookup_table.resize(timesteps);
    it = variable.FindMember("path");
    if (it == variable.end() && !(it->value.IsInt())) {
      throw JSON_error("dataset.variables[i].path", "string");
    }

    const string dataset_path = it->value.GetString();
    // TODO: handle data type other than uchar
    auto idx = string::npos;
    if (timesteps > 1) {
      idx = dataset_path.find("[step]");
      if (idx == string::npos) {
        throw runtime_error(
            "dataset path should contain [step] when timesteps > 1");
      }
    }
    for (size_t i = 0; i < timesteps; i++) {
      auto real_path = dataset_path;
      if (idx != std::string::npos) {
        // substitue [step]
        real_path.replace(idx, idx + 6, to_string(i));
      }

      Dataset dataset{};
      auto ext = get_file_extension(real_path);
      if (ext == ".raw") {
        if (!has_dimensions && !has_value_type) {
          throw runtime_error("dimensions and value type information is "
                              "needed for raw dataset.");
        }
        RawReader reader(real_path, info.dimensions, info.value_type);
        dataset = reader.load();
      } else if (ext == ".mrc") {
        MRCReader reader(real_path);
        dataset = reader.load();
      } else {
        throw runtime_error("unknown dataset format: " + ext);
      }
      datasets.emplace_back(move(dataset));
      timestep_lookup_table[i] = datasets.size() - 1;
    }
  }
}

auto DatasetStore::get(const std::string &name, const std::string &variable,
                       uint32_t timestep) const -> const voxer::Dataset & {
  const auto it = lookup_table.find(name);
  if (it == lookup_table.end()) {
    throw runtime_error("cannot find dataset");
  }

  auto variable_lookup_table = it->second;
  const auto v_it = variable_lookup_table.find(variable);
  if (v_it == variable_lookup_table.end()) {
    throw runtime_error("cannot find variable");
  }

  auto timestep_lookup_table = v_it->second;
  if (timestep > timestep_lookup_table.size()) {
    throw runtime_error("cannot find timestep");
  }

  auto idx = timestep_lookup_table[timestep];
  return datasets[idx];
}

auto DatasetStore::get_or_create(const SceneDataset &scene_dataset,
                                 const vector<SceneDataset> &scene_datasets)
    -> const voxer::Dataset & {
  if (!scene_dataset.diff) {
    return this->get(scene_dataset.name, scene_dataset.variable,
                     scene_dataset.timestep);
  }

  // TODO: ensure scene_dataset.parent < (current idx in scene_datasets), and
  // all previous datasets are processed
  auto &parent = scene_datasets[scene_dataset.parent];
  // no recursion
  auto &parent_dataset =
      this->get(parent.name, parent.variable, parent.timestep);

  // TODO: ensure scene_dataset.another < (current idx in scene_datasets), and
  // all previous datasets are processed
  auto &another = scene_datasets[scene_dataset.another];
  auto &another_dataset =
      this->get(another.name, another.variable, another.timestep);

  auto id = parent_dataset.id + "-" + another_dataset.id;
  if (temp_datasets.find(id) != temp_datasets.end()) {
    return temp_datasets.at(id);
  }

  auto differed_buffer = differ(parent_dataset.buffer, another_dataset.buffer);
  Dataset dataset{move(id), parent_dataset.info, move(differed_buffer)};
  temp_datasets.emplace(id, dataset);
  return temp_datasets.at(id);
}

auto DatasetStore::print() const -> string {
  if (lookup_table.empty()) {
    return "[]";
  }

  string res = "[";
  for (auto &item : lookup_table) {
    res += "{";
    auto &name = item.first;
    res += fmt::format(R"("name":"{}","variables":[)", name);
    auto &variable_lookup_table = item.second;
    if (variable_lookup_table.empty()) {
      res += "]},";
      continue;
    }

    for (auto &variable : variable_lookup_table) {
      auto &variable_name = variable.first;
      auto &timestep_lookup_table = variable.second;
      res += fmt::format(R"({{"name":"{}","timesteps":{}}},)", variable_name,
                         timestep_lookup_table.size());
    }
    res[res.find_last_of(',')] = ']';
    res += "},";
  }
  res[res.find_last_of(',')] = ']';
  return res;
}
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

  this->document.PushBack(current.Move(), document.GetAllocator());

  rapidjson::StringBuffer buffer{};
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  document.Accept(writer);

  ofstream fs(this->path);
  fs.write(buffer.GetString(), buffer.GetSize());
  fs.close();
}

void DatasetStore::load() {
  ifstream fs(this->path);
  if (!fs.good()) {
    ofstream ofs(this->path);
    ofs.write("[]", 2);
    ofs.close();
  }
  fs.close();
  this->load_from_file(this->path);
}

} // namespace voxer