#pragma once
#include <map>
#include <string>
#include <voxer/Scene.hpp>

class PipelineStore {
public:
  void load_from_file(const std::string &path);
  [[nodiscard]] auto get(const std::string &id) const -> const voxer::Scene &;
  [[nodiscard]] auto save(const std::string &json, voxer::Scene scene)
      -> std::string;

private:
  simdjson::ParsedJson pj;
  std::map<std::string, voxer::Scene> pipelines;
};
