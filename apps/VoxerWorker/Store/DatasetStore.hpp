#pragma once
#include "DataModel/DatasetInfo.hpp"
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer::remote {

class ManagerAPIClient;

class DatasetStore {
public:
  static DatasetStore &default_instance() noexcept;

  void set_storage(std::string storage_path) noexcept;

  void set_manager(ManagerAPIClient *manager) noexcept;

  [[nodiscard]] std::shared_ptr<StructuredGrid> get(const DatasetID &id);

  [[nodiscard]] std::shared_ptr<StructuredGrid>
  load(const std::string &id, const std::string &name, const std::string &path);

  void add(const DatasetID &id, const std::shared_ptr<StructuredGrid> &dataset);

private:
  DatasetStore() = default;

  ManagerAPIClient *m_manager = nullptr;
  std::string m_storage_path;

  std::mutex m_mutex;
  std::unordered_map<std::string, std::weak_ptr<StructuredGrid>> m_datasets;
};

} // namespace voxer::remote
