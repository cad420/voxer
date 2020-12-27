#include "Server/WebSockeRequestHandler.hpp"
#include <Poco/Net/NetException.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/Util/Application.h>

namespace voxer::remote {

WebSocketRequestHandler::WebSocketRequestHandler(DatasetStore *datasets)
    : m_datasets(datasets), m_service(std::make_unique<Service>(datasets)) {}

void WebSocketRequestHandler::handleRequest(
    Poco::Net::HTTPServerRequest &request,
    Poco::Net::HTTPServerResponse &response) {
  assert(m_service != nullptr);

  using WebSocket = Poco::Net::WebSocket;

  try {
    auto buffer_size = 4 * 1024 * 1024; // 4MB buffer
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[buffer_size]);
    int flags = 0;
    int size;
    auto should_close = false;

    WebSocket ws(request, response);
    auto one_hour = Poco::Timespan(0, 1, 0, 0, 0);
    ws.setReceiveTimeout(one_hour);

    auto handler = [&ws](const uint8_t *data, uint32_t size) {
      ws.sendFrame(data, size, WebSocket::FRAME_BINARY);
    };

    do {
      size = ws.receiveFrame(buffer.get(), buffer_size, flags);
      should_close = size <= 0 || ((flags & WebSocket::FRAME_OP_BITMASK) ==
                                   WebSocket::FRAME_OP_CLOSE);
      auto is_binary =
          (flags & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_BINARY;
      if (!should_close && is_binary) {
        m_service->on_message(reinterpret_cast<const uint8_t *>(buffer.get()),
                              size, handler);
      }
    } while (!should_close);
  } catch (Poco::Net::WebSocketException &exc) {
    spdlog::error(exc.what());
    switch (exc.code()) {
    case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
      response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
      // fallthrough
    case WebSocket::WS_ERR_NO_HANDSHAKE:
    case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
    case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
      response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
      response.setContentLength(0);
      response.send();
      break;
    }
  }
}

} // namespace voxer::remote