#include "ConfigManager.h"
#include "DatasetManager.h"
#include "ParallelRenderer/Encoder.h"
#include "Poco/Buffer.h"
#include "Poco/Format.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/URI.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/ServerApplication.h"
#include "Renderer.h"
#include <iostream>
#include <map>
#include <string>

using namespace std;
using Poco::format;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::ServerSocket;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

DatasetManager datasets;
ConfigManager configs;
Renderer renderer;
Encoder encoder;

class PageRequestHandler : public HTTPRequestHandler {
public:
  void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) {
    Application &app = Application::instance();
    response.setChunkedTransferEncoding(true);
    Poco::URI uri(request.getURI().c_str());
    vector<string> segments;
    uri.getPathSegments(segments);
    if (segments.size() == 0) {
      response.setContentType("text/html");
      auto &ostr = response.send();
      ostr << "Websocket Server has been started!";
    } else {
      auto id = segments[segments.size() - 1];
      try {
        auto &config = configs.get(id);
        auto params = config.GetObject();
        auto queryParams = uri.getQueryParameters();
        map<string, string> extraParams;
        for (auto &param : queryParams) {
          extraParams[param.first] = param.second;
        }
        response.setContentType("image/jpeg");
        auto data = renderer.render(config, &extraParams);
        auto imageData = params["image"].GetObject();
        auto imgSize = ospcommon::vec2ui(imageData["width"].GetInt(),
                                         imageData["height"].GetInt());
        if (extraParams.find("width") != extraParams.end()) {
          imgSize.x = stoi(extraParams["width"]);
        }
        if (extraParams.find("height") != extraParams.end()) {
          imgSize.y = stoi(extraParams["height"]);
        }
        auto img = encoder.encode(data, imgSize, "JPEG");
        response.sendBuffer(img.data(), img.size());
      } catch (string &exc) {
        response.setContentType("text/html");
        response.setStatus(HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        response.setReason("Not Found");
        auto &ostr = response.send();
        ostr << "404, Not Found";
      }
    }
  }
};

class WebSocketRequestHandler : public HTTPRequestHandler {
public:
  rapidjson::Document d;
  char buffer[1024 * 1024];
  void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) {
    Application &app = Application::instance();
    try {
      WebSocket ws(request, response);
      ws.setReceiveTimeout(Poco::Timespan(0, 2, 0, 0, 0));
      app.logger().information("WebSocket connection established.");
      int flags;
      int n;
      do {
        n = ws.receiveFrame(buffer, sizeof(buffer), flags);
        if ((flags & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_PING) {
          ws.sendFrame(buffer, n, WebSocket::FRAME_OP_PONG);
        }
        d.Parse(buffer, n);
        if (!d.HasMember("operation") || !d["operation"].IsString()) {
          auto msg = "Invalid operation";
          ws.sendFrame(msg, sizeof(msg));
        } else {
          auto operation = string(d["operation"].GetString());
          if (operation == "render") {
            try {
              if (!d.HasMember("params") || !d["params"].IsObject()) {
                auto msg = "Invalid params";
                ws.sendFrame(msg, sizeof(msg));
              }
              auto params = d["params"].GetObject();
              auto &rendererParams = params["image"];
              auto data = renderer.render(d["params"]);
              auto imgSize = ospcommon::vec2ui(params["width"].GetInt(),
                                               params["height"].GetInt());
              auto img = encoder.encode(data, imgSize, "JPEG");
              ws.sendFrame(img.data(), img.size(), WebSocket::FRAME_BINARY);
            } catch (string &exc) {
              ws.sendFrame(exc.c_str(), exc.size());
            }
          } else if (operation == "generate") {
            if (!d.HasMember("params") || !d["params"].IsObject()) {
              auto msg = "{\"type\": \"error\" , \"value\": \"Invalid params\"";
              ws.sendFrame(msg, sizeof(msg));
            }
            auto id = configs.save(d["params"]);
            auto msg = "{\"type\": \"url\" , \"value\":" + id;
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
  }
};

class RequestHandlerFactory : public HTTPRequestHandlerFactory {
public:
  HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &request) {
    Application &app = Application::instance();
    app.logger().information("Request from " +
                             request.clientAddress().toString() + ": " +
                             request.getMethod() + " " + request.getURI() +
                             " " + request.getVersion());
    if (request.find("Upgrade") != request.end() &&
        Poco::icompare(request["Upgrade"], "websocket") == 0)
      return new WebSocketRequestHandler;
    else
      return new PageRequestHandler;
  }
};

class VoVis : public ServerApplication {
public:
  VoVis() : _helpRequested(false) {}
  ~VoVis() {}
  string datasetFile;
  string configureFile;

protected:
  void initialize(Application &self) {
    loadConfiguration();
    int argc = self.argv().size();
    vector<const char *> argvec;
    argvec.reserve(argc);
    for (auto &arg : self.argv()) {
      if (arg == "--mpi") {
        argvec.push_back("--osp:mpi");
      } else {
        argvec.push_back(const_cast<char *>(arg.c_str()));
      }
    }
    const char **argv = &argvec[0];
    OSPError init_error = ospInit(&argc, argv);
    if (init_error != OSP_NO_ERROR) {
      logger().error("OSP Error");
      exit(1);
    }
    ServerApplication::initialize(self);
  }

  void uninitialize() { ServerApplication::uninitialize(); }

  void defineOptions(OptionSet &options) {
    ServerApplication::defineOptions(options);
    options.addOption(Option("mpi", "", "OSPRay MPI Offload Rendering Mode.")
                          .required(false)
                          .repeatable(false));
    options.addOption(Option("datasets", "", "volume datasets configure file.")
                          .required(true)
                          .argument("<the value>", true)
                          .repeatable(false));
    options.addOption(
        Option("configures", "", "visuailization configures save file.")
            .required(true)
            .argument("<the value>", true)
            .repeatable(false));
    options.addOption(
        Option("help", "h",
               "display help information on command line arguments")
            .required(false)
            .repeatable(false));
  }

  void handleOption(const string &name, const string &value) {
    ServerApplication::handleOption(name, value);
    if (name == "datasets") {
      datasetFile = value;
    } else if (name == "configures") {
      configureFile = value;
    } else if (name == "help")
      _helpRequested = true;
  }

  void displayHelp() {
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("Server Application of Volume Visualization.");
    helpFormatter.format(cout);
  }

  int main(const vector<string> &args) {
    if (_helpRequested) {
      displayHelp();
    } else {
      try {
        datasets.load(datasetFile);
        configs.load(configureFile);
      } catch (string &exc) {
        logger().error(exc);
        exit(1);
      }
      logger().information(to_string(datasets.size()) + " dataset loaded.");
      auto port = (unsigned short)config().getInt("WebSocketServer.port", 3000);
      ServerSocket svs(port);
      HTTPServer server(new RequestHandlerFactory, svs, new HTTPServerParams);
      server.start();
      logger().information("Server starts at port: " + to_string(port));
      waitForTerminationRequest();
      server.stop();
    }
    return Application::EXIT_OK;
  }

private:
  bool _helpRequested;
};

POCO_SERVER_MAIN(VoVis)