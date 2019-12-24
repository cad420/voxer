#include "voxer/DatasetStore.hpp"
#include "databases/MRC/MRCReader.hpp"
#include "databases/Raw/RawReader.hpp"
#include "voxer/filter/differ.hpp"
#include "voxer/utils.hpp"
#include <cassert>
#include <fmt/core.h>
#include <simdjson/jsonparser.h>
#include <simdjson/simdjson.h>
#include <stdexcept>

using namespace std;

namespace voxer {

void DatasetStore::load_from_file(const string &filepath) {
  ifstream fs(filepath);
  if (!fs.good() || !fs.is_open()) {
    throw runtime_error("cannot open file: " + filepath);
  }
  stringstream sstr;
  sstr << fs.rdbuf();
  auto json = sstr.str();
  this->load_from_json(json.c_str(), json.size());
}

void DatasetStore::load_from_json(const char *json, uint32_t size) {
  if (!pj.allocate_capacity(size)) {
    throw runtime_error("prepare parsing JSON failed");
  }

  const int res = simdjson::json_parse(json, size, pj);
  if (res != 0) {
    throw domain_error("Parse Error: " + simdjson::error_message(res));
  }

  simdjson::ParsedJson::Iterator pjh(pj);
  if (!pjh.is_ok()) {
    throw domain_error("invalid JSON");
  }

  if (!pjh.is_array() || !pjh.down()) {
    throw JSON_error("root", "array");
  }

  // datasets
  do {
    if (!pjh.is_object()) {
      throw JSON_error("datasets[i]", "object");
    }

    // key values
    pjh.move_to_key("name");
    string name = pjh.get_string();

    auto &variable_lookup_table = lookup_table[name];
    pjh.up();

    VolumeInfo info{};
    auto has_value_type = false;
    if (pjh.move_to_key("type")) {
      if (!pjh.is_string()) {
        throw JSON_error("type", "string");
      }
      string type_str = pjh.get_string();
      if (type_str == "float") {
        info.value_type = ValueType::FLOAT;
      }
      pjh.up();
      has_value_type = true;
    }

    auto has_dimensions = false;
    if (pjh.move_to_key("dimensions")) {
      if (!pjh.is_array()) {
        throw JSON_error("dimensions", "array");
      }
      if (!pjh.move_to_index(0) || !pjh.is_integer()) {
        throw JSON_error("dimensions[0]", "integer");
      }
      info.dimensions[0] = static_cast<uint16_t>(pjh.get_integer());
      pjh.up();
      if (!pjh.move_to_index(1) || !pjh.is_integer()) {
        throw JSON_error("dimensions[1]", "integer");
      }
      info.dimensions[1] = static_cast<uint16_t>(pjh.get_integer());
      pjh.up();
      if (!pjh.move_to_index(2) || !pjh.is_integer()) {
        throw JSON_error("dimensions[2]", "integer");
      }
      info.dimensions[2] = static_cast<uint16_t>(pjh.get_integer());
      pjh.up();

      has_dimensions = true;
      pjh.up();
    }

    if (!pjh.move_to_key("variables") || !pjh.is_array()) {
      throw JSON_error("datasets[i].variables", "array");
    }
    pjh.down();

    // into variables []
    do {
      if (!pjh.is_object()) {
        throw JSON_error("datasets[i].variables[j]", "object");
      }
      pjh.move_to_key("name");
      string variable_name = pjh.get_string();
      pjh.up();

      auto &timestep_lookup_table = variable_lookup_table[variable_name];

      if (!pjh.move_to_key("timesteps") || !pjh.is_integer()) {
        throw JSON_error("datasets[i].variables[j].timesteps", "integer");
      }
      const auto timesteps = static_cast<uint32_t>(pjh.get_integer());
      assert(timesteps >= 1);
      pjh.up();

      timestep_lookup_table.resize(timesteps);

      pjh.move_to_key("path");
      const string path = pjh.get_string();
      // TODO: handle data type other than uchar
      auto idx = string::npos;
      if (timesteps > 1) {
        idx = path.find("[step]");
        if (idx == string::npos) {
          throw runtime_error(
              "dataset path should contain [step] when timesteps > 1");
        }
      }
      for (size_t i = 0; i < timesteps; i++) {
        auto real_path = path;
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
      pjh.up();
    } while (pjh.next());
    pjh.up(); // in variables []
    pjh.up(); // in dataset {}
  } while (pjh.next());

  fmt::print("load {} datasets.\n", datasets.size());
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

} // namespace voxer