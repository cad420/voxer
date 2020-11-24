#include "Server/WebSockeRequestHandler.hpp"
#include <Poco/Net/NetException.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/Util/Application.h>

using Poco::Util::Application;

namespace voxer::remote {

WebSocketRequestHandler::WebSocketRequestHandler(
    std::unique_ptr<AbstractService> service) {
  assert(service != nullptr);
  m_service = move(service);
}

void WebSocketRequestHandler::handleRequest(
    Poco::Net::HTTPServerRequest &request,
    Poco::Net::HTTPServerResponse &response) {
  assert(m_service != nullptr);

  using WebSocket = Poco::Net::WebSocket;

  Application &app = Application::instance();
  try {
    char buffer[4096];
    int flags = 0;
    int len;

    WebSocket ws(request, response);
    auto one_hour = Poco::Timespan(0, 1, 0, 0, 0);
    ws.setReceiveTimeout(one_hour);
    do {
      len = ws.receiveFrame(buffer, sizeof(buffer), flags);
      m_service->on_message(
          buffer, len,
          [&ws](const uint8_t *message, uint32_t size, bool is_binary) {
            ws.sendFrame(message, size,
                         is_binary ? WebSocket::FRAME_BINARY
                                   : WebSocket::FRAME_TEXT);
          });
    } while (len > 0 && (flags & WebSocket::FRAME_OP_BITMASK) !=
                            WebSocket::FRAME_OP_CLOSE);
  } catch (Poco::Net::WebSocketException &exc) {
    app.logger().log(exc);
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