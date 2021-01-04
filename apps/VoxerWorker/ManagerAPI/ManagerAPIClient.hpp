#pragma once
#include "DataModel/DatasetLoadInfo.hpp"
#include "RPC/MessageQueue.hpp"
#include <Poco/Net/WebSocket.h>
#include <Poco/URI.h>
#include <atomic>
#include <sstream>
#include <string>
#include <thread>

namespace voxer::remote {

class ManagerAPIClient {
public:
  explicit ManagerAPIClient(std::string address);

  ~ManagerAPIClient();

  [[nodiscard]] const char *get_address() const noexcept;

  void register_worker();

  DatasetLoadInfo get_dataset_load_info(const std::string &id);

  // TODO: download from manager

  void shutdown();

private:
  std::string m_address;
  Poco::URI m_uri;
  MessageQueue *m_queue;
  std::thread m_thread{};
  std::unique_ptr<Poco::Net::WebSocket> m_ws = nullptr;

  template <typename ResultType> ResultType request(const std::string &path);
};

} // namespace voxer::remote