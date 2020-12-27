#pragma once
#include "DataModel/DatasetLoadInfo.hpp"
#include "ManagerAPI/WebSocketClient.hpp"
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/NullStream.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <fmt/core.h>
#include <seria/deserialize.hpp>
#include <seria/object.hpp>
#include <sstream>
#include <string>
#include <thread>

namespace voxer::remote {

class Service;
class DatasetStore;

class ManagerAPIClient {
public:
  explicit ManagerAPIClient();
  explicit ManagerAPIClient(std::string address);
  ~ManagerAPIClient();

  void set_datasets(DatasetStore *datasets);

  [[nodiscard]] const char *get_address() const noexcept {
    return m_address.c_str();
  }

  void register_worker();

  DatasetLoadInfo get_dataset_load_info(const std::string &id);

  // TODO: download from manager

private:
  std::string m_address;
  Poco::URI m_uri;
  std::unique_ptr<WebSocketClient> m_ws = nullptr;
  std::unique_ptr<Service> m_service;
  std::thread m_thread{};

  template <typename ResultType> ResultType request(const std::string &path);
};

} // namespace voxer::remote