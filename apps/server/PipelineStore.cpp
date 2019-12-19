#include "PipelineStore.hpp"
#include "utils.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <stdexcept>
#include <voxer/utils.hpp>

using namespace std;
using namespace voxer;

void PipelineStore::load_from_file(const std::string &filepath) {
  ifstream fs(filepath);
  if (!fs.good() || !fs.is_open()) {
    throw runtime_error("cannot open file: " + filepath);
  }

  stringstream sstr;
  sstr << fs.rdbuf();
  auto json = sstr.str();

  if (!pj.allocate_capacity(json.size())) {
    throw runtime_error("prepare parsing JSON failed");
  }

  const int res = simdjson::json_parse(json, pj);
  if (res != 0) {
    throw runtime_error("Error parsing " + filepath + " : " +
                        simdjson::error_message(res));
  }

  simdjson::ParsedJson::Iterator pjh(pj);
  if (!pjh.is_ok()) {
    throw runtime_error("invalid json");
  }

  if (!pjh.is_object()) {
    throw JSON_error("root", "object");
  }

  if (!pjh.move_to_key("id") || !pjh.is_string()) {
    throw JSON_error("id", "string");
  }

  string id = pjh.get_string();
  pjh.up();

  pjh.move_to_key("params");

  auto scene = voxer::Scene::deserialize(pjh);
  pipelines.emplace(move(id), move(scene));
}

auto PipelineStore::get(const std::string &id) const -> const voxer::Scene & {
  if (pipelines.find(id) == pipelines.end()) {
    throw runtime_error("cannot find pipeline with id " + id);
  }

  return pipelines.at(id);
}

auto PipelineStore::save(const std::string &json, voxer::Scene scene)
    -> string {
  auto id = nanoid();
  while (pipelines.find(id) != pipelines.end()) {
    id = nanoid();
  }

  string filename = id + ".json";
  ofstream fs(filename);
  if (!fs.good() || !fs.is_open()) {
    throw runtime_error("cannot open " + filename + " to backup pipeline.");
  }

  auto backup = fmt::format(R"({{"id":"{}","params":{}}})", id, json);
  fs.write(backup.c_str(), backup.size());
  fs.close();

  pipelines.emplace(id, move(scene));

  return id;
}

void PipelineStore::load_from_directory(const std::string &directory) {
  for (const auto &entry : filesystem::directory_iterator(directory)) {
    if (entry.is_directory()) {
      continue;
    }

    auto &filepath = entry.path();
    auto ext = filepath.extension().string();
    if (ext != ".json") {
      continue;
    }

    this->load_from_file(filepath);
  }
}
