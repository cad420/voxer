#include "Store/DatasetStore.hpp"
#include <spdlog/spdlog.h>
#include <string>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer::remote {

DatasetStore::DatasetStore(std::string storage_path)
    : m_storage_path{std::move(storage_path)} {
  if (m_storage_path[m_storage_path.size() - 1] != '/') {
    m_storage_path += '/';
  }
}

std::shared_ptr<StructuredGrid>
DatasetStore::load(const std::string &id, const std::string &name,
                   const std::string &filename) {
  std::unique_lock lock(m_mutex);

  auto &handle = m_datasets[id];

  auto dataset = handle.lock();
  if (dataset != nullptr) {
    return dataset;
  }

  auto path = m_storage_path + filename;

  try {
    dataset = StructuredGrid::Load(path.c_str());
  } catch (const std::exception &error) {
    spdlog::error(error.what());
    throw error;
  }

  spdlog::info("Load dataset: {}", name);

  return dataset;
}

std::shared_ptr<StructuredGrid> DatasetStore::get(const std::string &id) {
  std::shared_ptr<StructuredGrid> dataset;

  std::unique_lock lock(m_mutex);
  auto &handle = m_datasets[id];
  lock.unlock();

  dataset = handle.lock();
  if (dataset != nullptr) {
    return dataset;
  }

  if (m_manager == nullptr) {
    return nullptr;
  }

  auto load_info = m_manager->get_dataset_load_info(id);
  auto loaded = this->load(load_info.id, load_info.name, load_info.path);
  handle = loaded;

  return loaded;
}

void DatasetStore::set_manager(ManagerAPIClient *client) noexcept {
  m_manager = client;
}

void DatasetStore::add(const DatasetID &id,
                       const std::shared_ptr<StructuredGrid> &dataset) {
  std::unique_lock lock(m_mutex);
  m_datasets.emplace(id, dataset);
}

} // namespace voxer::remote