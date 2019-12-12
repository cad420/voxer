#pragma once
#include <map>
#include <string>
#include <voxer/Scene.hpp>

class PipelineStore {
  using PipelineSave = std::pair<std::string, voxer::Scene>;

public:
  void load_from_file(const std::string &path);
  [[nodiscard]] auto get(const std::string &id) const -> const PipelineSave &;
  auto save(PipelineSave save) -> std::string;

private:
  std::map<std::string, PipelineSave> pipelines;
  std::string path = "pipelines.json";
};
