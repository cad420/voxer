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

std::shared_ptr<StructuredGrid>
DatasetStore::load_from_json(const rapidjson::Value &json) {
  DatasetLoadInfo load_info{};
  seria::deserialize(load_info, json);

  return load(load_info.id, load_info.name, load_info.path);
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
  auto ext = get_file_extension(path);
  if (ext == ".raw") {
    RawReader reader(path.c_str());
    dataset = reader.load();
  } else if (ext == ".mrc") {
    MRCReader reader(path);
    dataset = reader.load();
  } else {
    throw runtime_error("unknown dataset format: " + ext);
  }

  fmt::print("Load dataset: " + name);

  return dataset;
}

auto DatasetStore::get(const DatasetLoadInfo &desc)
    -> shared_ptr<StructuredGrid> {
  return get(desc.id);
}

auto DatasetStore::get(const DatasetId &id) -> shared_ptr<StructuredGrid> {
  std::shared_ptr<StructuredGrid> dataset;

  std::unique_lock lock(m_mutex);
  auto &handle = m_datasets[id];
  lock.unlock();

  dataset = handle.lock();
  if (dataset != nullptr) {
    return dataset;
  }

  auto load_info = m_manager->get_dataset_info(id);
  auto loaded = this->load(load_info.id, load_info.name, load_info.path);
  handle = loaded;
  return loaded;
}

} // namespace voxer::remote