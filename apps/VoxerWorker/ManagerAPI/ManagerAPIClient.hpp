#pragma once
#include "DataModel/DatasetLoadInfo.hpp"
#include <Poco/URI.h>
#include <atomic>
#include <sstream>
#include <string>
#include <thread>

namespace voxer::remote {

class Service;
class DatasetStore;

class ManagerAPIClient {
public:
  explicit ManagerAPIClient(std::string address);

  ~ManagerAPIClient();

  void set_datasets(DatasetStore *datasets);

  [[nodiscard]] const char *get_address() const noexcept;

  void register_worker();

  DatasetLoadInfo get_dataset_load_info(const std::string &id);

  // TODO: download from manager

private:
  std::atomic<bool> m_done{};
  std::string m_address;
  Poco::URI m_uri;
  std::unique_ptr<Service> m_service;
  std::thread m_thread{};

  template <typename ResultType> ResultType request(const std::string &path);
};

} // namespace voxer::remote