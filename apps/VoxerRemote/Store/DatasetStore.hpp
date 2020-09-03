#pragma once
#include "DataModel/Dataset.hpp"
#include <array>
#include <memory>
#include <rapidjson/document.h>
#include <string>
#include <unordered_map>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer::remote {

class DatasetStore {
public:
  auto load_from_file(const std::string &filepath)
      -> std::vector<std::shared_ptr<voxer::StructuredGrid>>;
  auto load_from_json(const char *json, uint32_t size)
      -> std::vector<std::shared_ptr<voxer::StructuredGrid>>;
  auto load_one(const rapidjson::Value &json)
      -> std::shared_ptr<voxer::StructuredGrid>;
  void add_from_json(const char *text, uint32_t size);
  [[nodiscard]] auto get(const voxer::remote::Dataset &desc) const
      -> const std::shared_ptr<voxer::StructuredGrid> &;
  [[nodiscard]] auto get(const DatasetId &id) const
  -> const std::shared_ptr<voxer::StructuredGrid> &;
  //  [[nodiscard]] auto
  //  get_or_create(const voxer::remote::Dataset &scene_dataset,
  //                const std::vector<voxer::remote::Dataset> &scene_datasets)
  //      -> const voxer::remote::Dataset &;
  [[nodiscard]] auto get() const -> const
      std::unordered_map<DatasetId, std::shared_ptr<voxer::StructuredGrid>> & {
    return m_datasets;
  }
  //  [[nodiscard]] auto print() const -> std::string;

private:
  rapidjson::Document m_document;
  std::unordered_map<std::string, std::shared_ptr<voxer::StructuredGrid>>
      m_temp_datasets;
  std::unordered_map<DatasetId, std::shared_ptr<voxer::StructuredGrid>>
      m_datasets;
};

} // namespace voxer::remote
