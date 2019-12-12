#include "PipelineStore.hpp"
#include "utils.hpp"
#include <stdexcept>

using namespace std;

void PipelineStore::load_from_file(const std::string &filepath) {
  ifstream fs(filepath);
  if (!fs.is_open()) {
    throw runtime_error("cannot open file: " + filepath);
  }

  // TODO: read and parse pipelines from file

  this->path = filepath;
}

auto PipelineStore::get(const std::string &id) const -> const PipelineSave & {
  if (pipelines.find(id) == pipelines.end()) {
    throw runtime_error("cannot find pipeline with id " + id);
  }
  return pipelines.at(id);
}

auto PipelineStore::save(PipelineSave save) -> string {
  auto id = nanoid();
  while (pipelines.find(id) != pipelines.end()) {
    id = nanoid();
  }

  ofstream fs(path);
  // TODO: write pipelines to file
  fs.write(save.first.c_str(), save.first.size());
  fs.close();

  pipelines.emplace(id, move(save));

  return id;
}
