#include "voxer/data/Slice.hpp"
#include "voxer/encoders/Encoder.hpp"
#include "voxer/http/RequestHandler.hpp"
#include "voxer/managers/ConfigManager.hpp"
#include "voxer/renderers/Renderer.hpp"
#include <map>
#include <memory>

using namespace std;
using ospcommon::vec2i;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

extern ConfigManager configs;
extern DatasetManager datasets;
extern Encoder encoder;

void ImageRequestHandler::handleRequest(HTTPServerRequest &request,
                                        HTTPServerResponse &response) {
  response.setChunkedTransferEncoding(true);
  response.add("Access-Control-Allow-Origin", "*");
  response.add("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  response.add("Access-Control-Allow-Headers", "content-type");
  response.setContentType("image/jpeg");

  map<string, string> params;
  auto queryParams = this->uri.getQueryParameters();
  for (auto &param : queryParams) {
    params[param.first] = param.second;
  }

  try {
    Image img;
    ospcommon::vec2ui size;
    bool isRGBA = true;
    if (type == DataType::rendering) {
      auto id = segments[segments.size() - 1];
      auto &config = configs.get(id);
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
        isRGBA = false;
      } else {
        renderer.reset(new OSPRayRenderer());
      }
      auto data = renderer->render(config, size, cameraConfig);
      img = encoder.encode(data, size, "JPEG", isRGBA);
    } else if (type == DataType::slice) {
      auto id = segments[segments.size() - 1];
      auto &dataset = datasets.get(id);
      auto axis = Axis::x;
      if (params.find("axis") != params.end()) {
        if (params["axis"] == "1") {
          axis = Axis::y;
        } else if (params["axis"] == "2") {
          axis = Axis::z;
        }
      }
      auto offset =
          params.find("offset") != params.end() ? stoi(params["offset"]) : 0;
      img = createSlice(dataset, axis, offset);
      isRGBA = false;
    }
    response.sendBuffer(img.data(), img.size());
  } catch (string &exc) {
    response.setContentType("text/html");
    response.setStatus(HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
    response.setReason("Not Found");
    auto &ostr = response.send();
    ostr << "404, Not Found";
  }
};