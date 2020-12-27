#include "WebSocketClient.hpp"
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPMessage.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/WebSocket.h>
#include <spdlog/spdlog.h>

namespace voxer::remote {

WebSocketClient::~WebSocketClient() noexcept {
  try {
    if (m_ws != nullptr) {
      m_ws->close();
    }
  } catch (std::exception &error) {
    spdlog::error("WebSocketClient Error: {}", error.what());
  }
}

void WebSocketClient::connect(const char *host, uint16_t port,
                              const char *path) {
  if (m_handle_message == nullptr) {
    throw std::runtime_error("should register handler for WebSocket messages");
  }

  using namespace Poco::Net;

  Poco::Net::HTTPClientSession session(host, port);
  Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path,
                                 Poco::Net::HTTPMessage::HTTP_1_1);
  Poco::Net::HTTPResponse response;
  try {
    m_ws = std::make_unique<Poco::Net::WebSocket>(session, request, response);
    auto buffer_size = 4 * 1024 * 1024; // 4MB buffer
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[buffer_size]);
    int flags = 0;
    int length = 0;
    auto should_close = false;
    auto one_hour = Poco::Timespan(0, 1, 0, 0, 0);
    m_ws->setReceiveTimeout(one_hour);

    do {
      length = m_ws->receiveFrame(buffer.get(), buffer_size, flags);
      should_close = length < 0 || (flags & WebSocket::FRAME_OP_BITMASK) ==
                                       WebSocket::FRAME_OP_CLOSE;
      if (!should_close) {
        m_handle_message(buffer.get(), length,
                         (flags & WebSocket::FRAME_OP_BINARY) ==
                             WebSocket::FRAME_OP_BINARY);
      }
    } while (!should_close);
    spdlog::info("WebSocketClient connection closed");
  } catch (std::exception &error) {
    spdlog::error("WebSocketClient Connect Error: {}", error.what());
  }
}

void WebSocketClient::on_message(
    const WebSocketClient::MessageCallback &callback) {
  m_handle_message = callback;
}

void WebSocketClient::send(const uint8_t *message, uint32_t size,
                           bool is_binary) {
  using namespace Poco::Net;
  if (m_ws == nullptr)
    return;

  auto len = m_ws->sendFrame(message, size,
                             is_binary ? WebSocket::FRAME_BINARY
                                       : WebSocket::FRAME_TEXT);
  if (len == 0) {
    spdlog::warn("WebSocketClient sent message failed.");
  }
}

void WebSocketClient::close() {
  if (m_ws == nullptr)
    return;

  m_ws->close();
}

} // namespace voxer::remote
