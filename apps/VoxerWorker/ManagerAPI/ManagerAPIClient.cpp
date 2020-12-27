#include "ManagerAPI/ManagerAPIClient.hpp"
#include "RPC/Service.hpp"
#include "Store/DatasetStore.hpp"
#include <fmt/core.h>
#include <thread>

namespace voxer::remote {

ManagerAPIClient::ManagerAPIClient(std::string address)
    : m_address(std::move(address)), m_uri("http://" + m_address) {}

ManagerAPIClient::ManagerAPIClient() : ManagerAPIClient("127.0.0.1:3001") {}

ManagerAPIClient::~ManagerAPIClient() {
  m_ws->close();
  m_thread.join();
}

void ManagerAPIClient::set_datasets(DatasetStore *datasets) {
  m_service = std::make_unique<Service>(datasets);
  register_worker();
}

DatasetLoadInfo ManagerAPIClient::get_dataset_load_info(const std::string &id) {
  return request<DatasetLoadInfo>("/datasets/" + id);
}

void ManagerAPIClient::register_worker() {
  m_ws = std::make_unique<WebSocketClient>();
  m_ws->on_message([ws = m_ws.get(), service = m_service.get()](
                       uint8_t *message, uint32_t size, bool is_binary) {
    if (!is_binary || service == nullptr) {
      return;
    }

    service->on_message(
        message, size, [ws](const uint8_t *response, uint32_t total) {
          ws->send(reinterpret_cast<const uint8_t *>(response), total, true);
        });
  });

  m_thread = std::thread([ws = m_ws.get(), uri = m_uri]() {
    ws->connect(uri.getHost().c_str(), uri.getPort(), "/worker");
  });
}

template <typename ResultType>
ResultType ManagerAPIClient::request(const std::string &path) {
  using Poco::Net::HTTPClientSession;
  using Poco::Net::HTTPRequest;
  using Poco::Net::HTTPResponse;

  HTTPClientSession session(m_uri.getHost(), m_uri.getPort());
  HTTPRequest req(HTTPRequest::HTTP_GET, path,
                  Poco::Net::HTTPMessage::HTTP_1_1);
  HTTPResponse res;

  session.sendRequest(req);
  std::istream &rs = session.receiveResponse(res);
  auto status = res.getStatus();
  if (status != Poco::Net::HTTPResponse::HTTP_OK) {
    Poco::NullOutputStream null;
    Poco::StreamCopier::copyStream(rs, null);
    throw std::runtime_error(
        "Request failed, status: " +
        Poco::Net::HTTPResponse::getReasonForStatus(status));
  }

  std::stringstream raw_json;
  Poco::StreamCopier::copyStream(rs, raw_json);
  auto s = raw_json.str();

  rapidjson::Document document;
  document.Parse(s.c_str(), s.size());
  if (document.HasParseError()) {
    throw std::runtime_error("invalid response, should be a json");
  }

  if (!document.IsObject()) {
    throw std::runtime_error("invalid response, should be object");
  }

  auto value = document.GetObject();
  if (!value.HasMember("code") || !value["code"].IsInt()) {
    throw std::runtime_error("invalid response, response.code be an integer");
  }

  auto code = value["code"].GetInt();

  switch (code) {
  case 200: {
    ResultType data{};
    seria::deserialize(data, value["data"]);
    return data;
  }
  default: {
    if (value.HasMember("data") && value["data"].IsString()) {
      auto error =
          fmt::format("Error({}): {}", code, value["data"].GetString());
      throw std::runtime_error(error);
    }
    throw std::runtime_error(fmt::format("Error({})", code));
  }
  }
}

} // namespace voxer::remote