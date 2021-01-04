#include "ManagerAPI/ManagerAPIClient.hpp"
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPMessage.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/NullStream.h>
#include <Poco/StreamCopier.h>
#include <fmt/core.h>
#include <seria/deserialize/rapidjson.hpp>
#include <spdlog/spdlog.h>
#include <thread>

namespace voxer::remote {

ManagerAPIClient::ManagerAPIClient(std::string address)
    : m_address(std::move(address)), m_uri("http://" + m_address),
      m_queue(&MessageQueue::get_instance()),
      m_thread(std::thread([this]() { register_worker(); })) {}

ManagerAPIClient::~ManagerAPIClient() { shutdown(); }

const char *ManagerAPIClient::get_address() const noexcept {
  return m_address.c_str();
}

DatasetLoadInfo ManagerAPIClient::get_dataset_load_info(const std::string &id) {
  return request<DatasetLoadInfo>("/datasets/" + id);
}

void ManagerAPIClient::shutdown() {
  if (m_ws != nullptr) {
    m_ws->shutdown();
  }

  if (m_thread.joinable()) {
    m_thread.join();
  }
}

void ManagerAPIClient::register_worker() {
  using namespace Poco::Net;
  HTTPClientSession session(m_uri.getHost(), m_uri.getPort());
  HTTPRequest request(HTTPRequest::HTTP_GET, "/worker", HTTPMessage::HTTP_1_1);
  HTTPResponse response;

  try {
    auto buffer_size = 4 * 1024 * 1024; // 4MB buffer
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[buffer_size]);
    uint32_t flags = 0;
    int received;
    bool should_close;

    m_ws = std::make_unique<WebSocket>(session, request, response);

    spdlog::info("WebSocketClient connected to manager.");

    auto one_hour = Poco::Timespan(0, 1, 0, 0, 0);
    m_ws->setReceiveTimeout(one_hour);

    auto handler = [ws = m_ws.get()](const uint8_t *response, uint32_t total) {
      ws->sendFrame(reinterpret_cast<const uint8_t *>(response), total,
                    WebSocket::FRAME_BINARY);
    };

    do {
      received = m_ws->receiveFrame(buffer.get(), buffer_size,
                                    reinterpret_cast<int &>(flags));

      auto is_ping =
          (flags & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_PING;
      if (is_ping) {
        m_ws->sendFrame(buffer.get(), received,
                        WebSocket::FRAME_FLAG_FIN | WebSocket::FRAME_OP_PONG);
        continue;
      }

      should_close = received <= 0 || (flags & WebSocket::FRAME_OP_BITMASK) ==
                                          WebSocket::FRAME_OP_CLOSE;
      if (should_close) {
        break;
      }

      auto is_binary =
          (flags & WebSocket::FRAME_OP_BINARY) == WebSocket::FRAME_OP_BINARY;
      if (!is_binary || m_queue == nullptr) {
        continue;
      }

      m_queue->add_message(buffer.get(), received, handler);
    } while (true);
    spdlog::info("WebSocketClient connection closed");
  } catch (std::exception &error) {
    spdlog::error("WebSocketClient Error: {}", error.what());
  }
}

template <typename ResultType>
ResultType ManagerAPIClient::request(const std::string &path) {
  using Poco::Net::HTTPClientSession;
  using Poco::Net::HTTPRequest;
  using Poco::Net::HTTPResponse;

  try {
    HTTPClientSession session(m_uri.getHost(), m_uri.getPort());
    HTTPRequest req(HTTPRequest::HTTP_GET, path,
                    Poco::Net::HTTPMessage::HTTP_1_1);
    HTTPResponse res;

    session.sendRequest(req);
    spdlog::debug("Send request to manager, path: {}", path);

    std::istream &rs = session.receiveResponse(res);
    auto status = res.getStatus();
    if (status != Poco::Net::HTTPResponse::HTTP_OK) {
      Poco::NullOutputStream null;
      Poco::StreamCopier::copyStream(rs, null);
      throw std::runtime_error(
          "Request failed, status: " +
          Poco::Net::HTTPResponse::getReasonForStatus(status));
    }

    spdlog::debug("receive response from manager, path: {}", path);

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
  } catch (std::exception &error) {
    spdlog::error("Request to manager error: ", error.what());
    throw error;
  }
}

} // namespace voxer::remote