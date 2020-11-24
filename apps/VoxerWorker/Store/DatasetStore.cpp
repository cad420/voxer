#include "Store/DatasetStore.hpp"
#include "DataModel/DatasetLoadInfo.hpp"
#include "ManagerAPI/ManagerAPIClient.hpp"
#include <fmt/core.h>
#include <seria/deserialize.hpp>
#include <stdexcept>
#include <string>
#include <voxer/Data/StructuredGrid.hpp>
#include <voxer/Filters/differ.hpp>
#include <voxer/IO/MRCReader.hpp>
#include <voxer/IO/RawReader.hpp>
#include <voxer/IO/utils.hpp>

using namespace std;

namespace voxer::remote {

DatasetStore::DatasetStore(ManagerAPIClient *manager, string storage_path)
    : m_manager(manager), m_storage_path{std::move(storage_path)} {
  if (m_storage_path[m_storage_path.size() - 1] != '/') {
    m_storage_path += '/';
  }
}

auto DatasetStore::add_from_json(const rapidjson::Value &json) -> Iterator {
  DatasetLoadInfo load_info{};
  seria::deserialize(load_info, json);

  return add(load_info.id, load_info.name, load_info.path);
}

auto DatasetStore::add(const std::string &id, const std::string &name,
                       const std::string &filename) -> Iterator {
  std::promise<std::shared_ptr<StructuredGrid>> p;

  std::unique_lock lock(m_mutex);
  auto result = m_datasets.emplace(id, p.get_future());
  lock.unlock();

  auto path = m_storage_path + filename;
  shared_ptr<StructuredGrid> dataset{};
  auto ext = get_file_extension(path);
  if (ext == ".raw") {
    RawReader reader(path.c_str());
    dataset = reader.load();
  } else if (ext == ".mrc") {
    MRCReader reader(path);
    dataset = reader.load();
  } else {
    lock.lock();
    m_datasets.erase(id);
    throw runtime_error("unknown dataset format: " + ext);
  }

  fmt::print("Load dataset: " + name);

  p.set_value(dataset);
  return result.first;
}

auto DatasetStore::get(const DatasetLoadInfo &desc)
    -> shared_ptr<StructuredGrid> {
  return get(desc.id);
}

auto DatasetStore::get(const DatasetId &id) -> shared_ptr<StructuredGrid> {
  std::unique_lock lock(m_mutex);
  auto it = m_datasets.find(id);
  if (it != m_datasets.end()) {
    lock.unlock();
    return it->second.get();
  }
  lock.unlock();

  auto dataset = m_manager->get_dataset_info(id);
  it = this->add(dataset.id, dataset.name, dataset.path);
  return it->second.get();
}

bool DatasetStore::has(const string &id) {
  std::lock_guard lock(m_mutex);
  return m_datasets.count(id);
}

} // namespace voxer::remote