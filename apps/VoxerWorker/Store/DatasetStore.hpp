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

  bool has(const std::string &id);

  [[nodiscard]] std::shared_ptr<StructuredGrid>
  get(const DatasetLoadInfo &desc);
  [[nodiscard]] std::shared_ptr<StructuredGrid> get(const DatasetId &id);

private:
  ManagerAPIClient *m_manager;
  std::string m_storage_path;

  std::mutex m_mutex;
  std::unordered_map<DatasetId,
                     std::shared_future<std::shared_ptr<StructuredGrid>>>
      m_datasets;
  using Iterator = decltype(m_datasets)::iterator;

  Iterator add(const std::string &id, const std::string &name,
               const std::string &path);

  Iterator add_from_json(const rapidjson::Value &json);
};

} // namespace voxer::remote
