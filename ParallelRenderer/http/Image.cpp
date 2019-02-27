#include "ParallelRenderer/ConfigManager.h"
#include "ParallelRenderer/Encoder.h"
#include "ParallelRenderer/Renderer.h"
#include "RequestHandler.h"
#include <map>
#include <memory>

using namespace std;
using ospcommon::vec2i;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

extern ConfigManager configs;
extern Encoder encoder;

void ImageRequestHandler::handleRequest(HTTPServerRequest &request,
                                        HTTPServerResponse &response) {
  response.setChunkedTransferEncoding(true);
  response.add("Access-Control-Allow-Origin", "*");
  response.add("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  response.add("Access-Control-Allow-Headers", "content-type");

  if (segments.size() == 0) {
    response.setContentType("text/html");
    auto &ostr = response.send();
    ostr << "Websocket Server has been started!";
    return;
  }

  if (segments.size() >= 1) {
    auto id = segments[segments.size() - 1];
    auto &config = configs.get(id);
    if (segments.size() == 1) {
      response.setContentType("image/jpeg");
      try {
        map<string, string> params;
        for (auto &param : this->uri.getQueryParameters()) {
          params[param.first] = param.second;
        }
        CameraConfig cameraConfig(config.cameraConfig, params);
        vec2i size = config.size;
        if (params.find("width") != params.end()) {
          size.x = stoi(params["width"]);
        }
        if (params.find("height") != params.end()) {
          size.y = stoi(params["height"]);
        }

        unique_ptr<Renderer> renderer;
        if (config.volumesToRender.size() > 1) {
          renderer.reset(new VTKRenderer());
        } else {
          renderer.reset(new OSPRayRenderer());
        }
        auto data = renderer->render(config, size, cameraConfig);
        auto img = encoder.encode(data, size, "JPEG");
        response.sendBuffer(img.data(), img.size());
      } catch (string &exc) {
        response.setContentType("text/html");
        response.setStatus(HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        response.setReason("Not Found");
        auto &ostr = response.send();
        ostr << "404, Not Found";
      }
    } else {
      const auto resource = segments[1];
      if (resource == "histogram") {
        response.setContentType("application/json");
      }
    }
  }
};