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
  auto str = sstr.str();

  document.Parse(str.c_str(), str.size());
  if (!document.IsObject()) {
    throw JSON_error("root", "object");
  }

  auto json = document.GetObject();
  auto it = json.FindMember("id");
  if (it == json.end() || !(it->value.IsString())) {
    throw JSON_error("id", "string");
  }

  string id = it->value.GetString();

  it = json.FindMember("params");
  if (it == json.end() || !(it->value.IsObject())) {
    throw JSON_error("params", "object");
  }

  auto scene = voxer::Scene::deserialize(it->value);
  pipelines.emplace(id, move(scene));
  serialized.emplace(move(id), move(str));
}

auto PipelineStore::get(const std::string &id) const -> const voxer::Scene & {
  auto position = pipelines.find(id);
  if (position == pipelines.end()) {
    throw runtime_error("cannot find pipeline with id " + id);
  }

  return position->second;
}

auto PipelineStore::get_serialized(const std::string &id) const
    -> const std::string & {
  auto position = serialized.find(id);
  if (position == serialized.end()) {
    throw runtime_error("cannot find pipeline with id " + id);
  }

  return position->second;
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
  serialized.emplace(id, move(backup));

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

auto PipelineStore::print() const -> std::string {
  if (serialized.empty()) {
    return "[]";
  }

  string res = "[";
  for (auto &item : serialized) {
    res += item.second;
    res += ",";
  }
  res[res.find_last_of(',')] = ']';
  return res;
}
