#pragma once
#include <map>
#include <string>
#include <voxer/Scene.hpp>

class PipelineStore {
public:
  void load_from_file(const std::string &path);
  void load_from_directory(const std::string &directory);
  [[nodiscard]] auto get(const std::string &id) const -> const voxer::Scene &;
  [[nodiscard]] auto get_serialized(const std::string &id) const
      -> const std::string &;
  [[nodiscard]] auto save(const std::string &json, voxer::Scene scene)
      -> std::string;
  [[nodiscard]] auto print() const -> std::string;

private:
  rapidjson::Document document;
  std::map<std::string, voxer::Scene> pipelines;
  std::map<std::string, std::string> serialized;
};
