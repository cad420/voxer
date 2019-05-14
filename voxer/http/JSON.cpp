#include "third_party/rapidjson/document.h"
#include "voxer/data/Scatter.hpp"
#include "voxer/encoders/Encoder.hpp"
#include "voxer/http/RequestHandler.hpp"
#include "voxer/managers/ConfigManager.hpp"
#include "voxer/renderers/Renderer.hpp"
#include <iomanip>
#include <map>

using namespace std;
using ospcommon::vec2i;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

extern DatasetManager datasets;
extern Encoder encoder;

void JSONRequestHandler::handleRequest(HTTPServerRequest &request,
                                       HTTPServerResponse &response) {
  response.setChunkedTransferEncoding(true);
  response.add("Access-Control-Allow-Origin", "*");
  response.add("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  response.add("Access-Control-Allow-Headers", "content-type");

  rapidjson::Document d;
  rapidjson::Value a;
  auto &allocator = d.GetAllocator();

  if (type == DataType::histogram) {
    if (segments.size() != 2) {
      // TODO
    }
    auto id = segments[1];
    auto &dataset = datasets.get(id);
    auto &histogram = dataset.histogram;

    a.SetArray();
    for (int i = 0; i <= 255; i++) {
      a.PushBack(histogram[i], allocator);
    }
  } else if (type == DataType::scatter) {
    if (segments.size() != 3) {
      // TODO
    }

    auto id = segments[1];
    auto &datasetA = datasets.get(id);
    id = segments[2];
    auto &datasetB = datasets.get(id);

    // auto scale = segments[3];

    if (datasetA.dimensions != datasetB.dimensions) {
    }

    auto scatter =
        createScatter(datasetA.buffer, datasetB.buffer, datasetA.dimensions);
    a.SetArray();
    auto max = scatter.max;

    for (int i = 0; i <= 255; i++) {
      for (int j = 0; j <= 255; j++) {
        auto &count = scatter.points[i][j];
        if (count > 0) {
          rapidjson::Value v(rapidjson::kObjectType);
          stringstream ss;
          ss.precision(5);
          ss << (float)count;
          rapidjson::Value sv(ss.str().c_str(), allocator);
          v.AddMember("x", i, allocator);
          v.AddMember("y", j, allocator);
          v.AddMember("v", sv, allocator);
          a.PushBack(v, allocator);
        }
      }
    }
  }

  rapidjson::StringBuffer buffer;
  buffer.Clear();
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  a.Accept(writer);

  auto res = string(buffer.GetString());
  response.setContentType("application/json");
  auto &ostr = response.send();
  ostr << res;
};