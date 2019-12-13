#include "voxer/DatasetStore.hpp"
#include <cassert>
#include <fmt/core.h>
#include <simdjson/jsonparser.h>
#include <simdjson/simdjson.h>
#include <stdexcept>
#include <voxer/utils.hpp>

using namespace std;

namespace voxer {

void DatasetStore::load_from_file(const string &filepath) {
  this->load_from_json(filepath.c_str(), filepath.size());
}

void DatasetStore::load_from_json(const char *json, uint32_t size) {
  auto p = simdjson::get_corpus(json);
  if (!pj.allocate_capacity(p.size())) {
    throw runtime_error("prepare parsing JSON failed");
  }

  const int res = simdjson::json_parse(p, pj);
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

    FieldInfo meta{};
    pjh.move_to_key("type");
    const string type = pjh.get_string();
    if (type == "float") {
      meta.type = DatasetValueType::FLOAT;
      meta.type_size = sizeof(float);
    }
    pjh.up();

    if (!pjh.move_to_key("dimensions") || !pjh.is_array()) {
      throw JSON_error("dimensions", "array");
    }
    if (!pjh.move_to_index(0) || !pjh.is_integer()) {
      throw JSON_error("dimensions[0]", "integer");
    }
    meta.dimensions[0] = static_cast<uint32_t>(pjh.get_integer());
    pjh.up();
    if (!pjh.move_to_index(1) || !pjh.is_integer()) {
      throw JSON_error("dimensions[1]", "integer");
    }
    meta.dimensions[1] = static_cast<uint32_t>(pjh.get_integer());
    pjh.up();
    if (!pjh.move_to_index(2) || !pjh.is_integer()) {
      throw JSON_error("dimensions[2]", "integer");
    }
    meta.dimensions[2] = static_cast<uint32_t>(pjh.get_integer());
    pjh.up();
    pjh.up();

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
      auto total = meta.dimensions[0] * meta.dimensions[1] *
                   meta.dimensions[2] * meta.type_size;
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

        ifstream fs(real_path.c_str(), ios::binary);
        fs.unsetf(ios::skipws);
        if (!fs.is_open()) {
          throw runtime_error(fmt::format(
              "Read {} timestep of volume data {}'s variable {} failed.",
              to_string(i + 1), name, variable_name));
        }

        // load
        Dataset dataset{meta};
        dataset.buffer.reserve(total);
        dataset.buffer.insert(dataset.buffer.begin(),
                              istream_iterator<uint8_t>(fs),
                              istream_iterator<uint8_t>());
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

auto DatasetStore::get(const SceneDataset &scene_dataset) const
    -> const voxer::Dataset & {
  return this->get(scene_dataset.name, scene_dataset.variable,
                   scene_dataset.timestep);
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