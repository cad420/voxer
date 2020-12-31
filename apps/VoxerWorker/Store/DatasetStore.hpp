#pragma once
#include "ManagerAPI/ManagerAPIClient.hpp"
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer::remote {

class DatasetStore {
public:
  explicit DatasetStore(std::string storage_path);

  void set_manager(ManagerAPIClient *manager) noexcept;

  [[nodiscard]] std::shared_ptr<StructuredGrid> get(const DatasetID &id);

  [[nodiscard]] std::shared_ptr<StructuredGrid>
  load(const std::string &id, const std::string &name, const std::string &path);

  void add(const DatasetID &id, const std::shared_ptr<StructuredGrid> &dataset);

private:
  ManagerAPIClient *m_manager = nullptr;
  std::string m_storage_path;

  std::mutex m_mutex;
  std::unordered_map<std::string, std::weak_ptr<StructuredGrid>> m_datasets;
};

} // namespace voxer::remote
