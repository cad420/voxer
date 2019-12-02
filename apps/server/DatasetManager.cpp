#include "DatasetManager.hpp"
#include "utils.hpp"
#include <cassert>
#include <fmt/core.h>
#include <simdjson/jsonparser.h>
#include <simdjson/simdjson.h>
#include <stdexcept>

using namespace std;
using namespace voxer;

void DatasetManager::load(const string &filepath) {
  auto p = simdjson::get_corpus(filepath);
  if (!pj.allocate_capacity(p.size())) {
    throw runtime_error("");
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
    auto dataset = make_shared<Dataset>();

    // key values
    pjh.move_to_key("name");
    dataset->name = pjh.get_string();
    pjh.up();

    pjh.move_to_key("type");
    const string type = pjh.get_string();
    if (type == "float") {
      dataset->type = Dataset::ValueType::FLOAT;
      dataset->type_size = sizeof(float);
    } else if (type == "uchar") {
      dataset->type = Dataset::ValueType::UCHAR;
    }
    pjh.up();

    if (!pjh.move_to_key("dimensions") || !pjh.is_array()) {
      throw JSON_error("dimensions", "array");
    }
    if (!pjh.move_to_index(0) || !pjh.is_integer()) {
      throw JSON_error("dimensions[0]", "integer");
    }
    const auto x = static_cast<uint32_t>(pjh.get_integer());
    pjh.up();
    if (!pjh.move_to_index(1) || !pjh.is_integer()) {
      throw JSON_error("dimensions[1]", "integer");
    }
    const auto y = static_cast<uint32_t>(pjh.get_integer());
    pjh.up();
    if (!pjh.move_to_index(2) || !pjh.is_integer()) {
      throw JSON_error("dimensions[2]", "integer");
    }
    const auto z = static_cast<uint32_t>(pjh.get_integer());
    dataset->dimensions = {x, y, z};
    pjh.up();
    pjh.up();

    if (!pjh.move_to_key("variables") || !pjh.is_array()) {
      throw JSON_error("datasets[i].variables", "array");
    }
    pjh.down();

    // into variables []
    do {
      DatasetVariable variable;
      if (!pjh.is_object()) {
        throw JSON_error("datasets[i].variables[j]", "object");
      }
      pjh.move_to_key("name");
      variable.name = pjh.get_string();
      pjh.up();

      if (!pjh.move_to_key("timesteps") || !pjh.is_integer()) {
        throw JSON_error("datasets[i].variables[j].timesteps", "integer");
      }
      const auto timesteps = static_cast<uint32_t>(pjh.get_integer());
      variable.timesteps.reserve(timesteps);
      assert(timesteps >= 1);
      pjh.up();

      pjh.move_to_key("path");
      const string path = pjh.get_string();
      auto total = dataset->dimensions[0] * dataset->dimensions[1] *
                   dataset->dimensions[2] * dataset->type_size;
      if (timesteps == 1) {
        // load path directly
        Buffer buffer(total);
        auto file = fopen(path.c_str(), "rb");
        size_t read = fread(buffer.data(), dataset->type_size, total, file);
        if (read != total) {
          throw runtime_error(
              fmt::format("Read volume data {}'s variable {} failed.",
                          dataset->name, variable.name));
        }
        variable.timesteps.emplace_back(move(buffer));
      } else {
        // substitue [step]
        auto idx = path.find("[step]");
        if (idx == std::string::npos) {
          throw runtime_error("when a variable has multiple timesteps, it's "
                              "path should have `[step]` placeholder.");
        }
        for (auto i = 0ull; i < timesteps; i++) {
          // load
          auto real_path = path;
          real_path.replace(idx, idx + 6, to_string(i));
          Buffer buffer(total);
          auto file = fopen(real_path.c_str(), "rb");
          size_t read = fread(buffer.data(), dataset->type_size, total, file);
          if (read != total) {
            throw runtime_error(fmt::format(
                "Read {} timestep of volume data {}'s variable {} failed.",
                to_string(i + 1), dataset->name, variable.name));
          }
          variable.timesteps.emplace_back(move(buffer));
        }
      }
      pjh.up();

      dataset->variables[variable.name] = move(variable);
    } while (pjh.next());
    pjh.up(); // in variables []
    pjh.up(); // in dataset {}

    datasets[dataset->name] = move(dataset);
  } while (pjh.next());
}

auto DatasetManager::print() const -> string {
  string res = "[";
  for (auto &item : datasets) {
    const auto &dataset = *item.second;
    res += fmt::to_string(dataset);
    res += ",";
  }
  res[res.find_last_of(',')] = ']';
  return res;
}

auto DatasetManager::get(const std::string &name)
    -> std::shared_ptr<voxer::Dataset> {
  const auto it = datasets.find(name);
  if (it == datasets.end()) {
    return nullptr;
  }
  return it->second;
}
