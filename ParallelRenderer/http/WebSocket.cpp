#include "Poco/Net/WebSocket.h"
#include "ParallelRenderer/ConfigManager.h"
#include "ParallelRenderer/Encoder.h"
#include "ParallelRenderer/Renderer.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Util/ServerApplication.h"
#include "RequestHandler.h"
#include <map>
#include <memory>

using namespace std;
using ospcommon::vec2i;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
using Poco::Util::Application;

extern ConfigManager configs;
extern ConfigManager configs;
extern Encoder encoder;

void WebSocketRequestHandler::handleRequest(HTTPServerRequest &request,
                                            HTTPServerResponse &response) {
  Application &app = Application::instance();
  try {
    WebSocket ws(request, response);
    ws.setReceiveTimeout(Poco::Timespan(0, 2, 0, 0, 0));
    app.logger().information("WebSocket connection established.");
    int flags;
    int n;
    do {
      n = ws.receiveFrame(buffer, sizeof(buffer), flags);
      if (n <= 0)
        continue;
      if ((flags & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_PING) {
        ws.sendFrame(buffer, n, WebSocket::FRAME_OP_PONG);
      }
      d.Parse(buffer, n);
      if (!d.HasMember("operation") || !d["operation"].IsString()) {
        string msg = "Invalid operation";
        ws.sendFrame(msg.c_str(), msg.size());
      } else {
        auto operation = string(d["operation"].GetString());
        if (operation == "render") {
          try {
            if (!d.HasMember("params") || !d["params"].IsObject()) {
              auto msg = "Invalid params";
              ws.sendFrame(msg, sizeof(msg));
            }
            auto params = d["params"].GetObject();
            auto config = configs.create(d["params"]);

            unique_ptr<Renderer> renderer;
            auto isRGBA = true;
            if (config.volumesToRender.size() > 1) {
              renderer.reset(new VTKRenderer());
              isRGBA = false;
            } else {
              renderer.reset(new OSPRayRenderer());
            }
            auto data = renderer->render(config);
            auto img = encoder.encode(data, config.size, "JPEG", isRGBA);
            ws.sendFrame(img.data(), img.size(), WebSocket::FRAME_BINARY);
          } catch (string &exc) {
            auto msg = "{\"type\": \"error\" , \"value\": \"" + exc + "\"}";
            ws.sendFrame(msg.c_str(), msg.size());
          }
        } else if (operation == "generate") {
          if (!d.HasMember("params") || !d["params"].IsObject()) {
            auto msg = "{\"type\": \"error\" , \"value\": \"Invalid params\"}";
            ws.sendFrame(msg, sizeof(msg));
          }
          auto id = configs.save(d["params"]);
          auto msg = "{\"type\": \"config\" , \"value\":\"" + id + "\"}";
          ws.sendFrame(msg.c_str(), msg.size());
        }
      }
    } while (n > 0 && (flags & WebSocket::FRAME_OP_BITMASK) !=
                          WebSocket::FRAME_OP_CLOSE);
    app.logger().information("WebSocket connection closed.");
  } catch (WebSocketException &exc) {
    app.logger().log(exc);
    switch (exc.code()) {
    case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
      response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
    case WebSocket::WS_ERR_NO_HANDSHAKE:
    case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
    case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
      response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
      response.setContentLength(0);
      response.send();
      break;
    }
  } catch (const exception &exc) {
    app.logger().warning(exc.what());
  }
};