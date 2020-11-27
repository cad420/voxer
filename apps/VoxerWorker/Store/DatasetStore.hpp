#pragma once
#include "DataModel/DatasetLoadInfo.hpp"
#include <ManagerAPI/ManagerAPIClient.hpp>
#include <array>
#include <future>
#include <memory>
#include <mutex>
#include <rapidjson/document.h>
#include <string>
#include <unordered_map>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer::remote {

class DatasetStore {
public:
  DatasetStore(ManagerAPIClient *client, std::string storage_path);

  [[nodiscard]] std::shared_ptr<StructuredGrid>
  get(const DatasetLoadInfo &desc);
  [[nodiscard]] std::shared_ptr<StructuredGrid> get(const DatasetId &id);

  [[nodiscard]] std::shared_ptr<StructuredGrid>
  get(const DatasetId &id, const std::string &name, const std::string &path);

private:
  ManagerAPIClient *m_manager;
  std::string m_storage_path;

  std::mutex m_mutex;
  std::unordered_map<DatasetId, std::weak_ptr<StructuredGrid>> m_datasets;

  using Iterator = decltype(m_datasets)::iterator;

  std::shared_ptr<StructuredGrid>
  load(const std::string &id, const std::string &name, const std::string &path);

  std::shared_ptr<StructuredGrid> load_from_json(const rapidjson::Value &json);
};

} // namespace voxer::remote
