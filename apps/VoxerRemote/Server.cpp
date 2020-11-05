#include "Server.hpp"
#include <Poco/Net/NetException.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/Util/Application.h>

using Poco::Util::Application;

namespace voxer::remote {

void DefaultRequestHandler::handleRequest(
    Poco::Net::HTTPServerRequest &request,
    Poco::Net::HTTPServerResponse &response) {
  auto &uri = request.getURI();
  response.setContentType("text/plain");
  if (uri != "/") {
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    std::ostream &ostr = response.send();
    ostr << "Not Found.";
    return;
  }

  std::ostream &ostr = response.send();
  ostr << "Welcome to voxer.";
}

void RPCRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                                      Poco::Net::HTTPServerResponse &response) {
  std::istream &istr = request.stream();
  int len = request.getContentLength();

  std::unique_ptr<char[]> buffer;
  if (len > 0) {
    buffer.reset(new char[len]);
  } else {
    len = 0;
  }

  istr.read(buffer.get(), len);
  response.setKeepAlive(false);
  response.setContentType("application/json");
  m_service->on_message(
      buffer.get(), len,
      [&response](const uint8_t *message, uint32_t size, bool is_binary) {
        response.setContentLength(size);
        std::ostream &ostr = response.send();
        ostr.write(reinterpret_cast<const char *>(message), size);
      });
}

RPCRequestHandler::RPCRequestHandler(AbstractService *service) {
  assert(service != nullptr);
  m_service = service;
}

WebSocketRequestHandler::WebSocketRequestHandler(AbstractService *service) {
  m_service = service;
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

Poco::Net::HTTPRequestHandler *
MyHTTPRequestHandlerFactory::createRequestHandler(
    const Poco::Net::HTTPServerRequest &request) {
  auto &uri = request.getURI();

  for (auto &service : services) {
    auto path = service->get_path();
    if (uri != path) {
      continue;
    }

    if (service->get_protocol() == AbstractService::Protocol::RPC) {
      return new RPCRequestHandler(service.get());
    } else if (service->get_protocol() ==
               AbstractService::Protocol::WebSocket) {
      return new WebSocketRequestHandler(service.get());
    }
  }

  return new DefaultRequestHandler();
}

void MyHTTPRequestHandlerFactory::add_service(
    std::unique_ptr<AbstractService> &&service) noexcept {
  services.emplace_back(move(service));
}

} // namespace voxer::remote