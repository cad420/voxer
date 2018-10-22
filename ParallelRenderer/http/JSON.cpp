#include "ParallelRenderer/ConfigManager.h"
#include "ParallelRenderer/Encoder.h"
#include "ParallelRenderer/Renderer.h"
#include "RequestHandler.h"
#include "third_party/rapidjson/document.h"
#include <map>

using namespace std;
using ospcommon::vec2i;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

extern DatasetManager datasets;
extern Renderer renderer;
extern Encoder encoder;

void JSONRequestHandler::handleRequest(HTTPServerRequest &request,
                                       HTTPServerResponse &response) {
  response.setChunkedTransferEncoding(true);
  response.add("Access-Control-Allow-Origin", "*");
  response.add("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  response.add("Access-Control-Allow-Headers", "content-type");

  auto id = segments[0];
  auto &dataset = datasets.get(id);
  auto &histogram = dataset.histogram;

  rapidjson::Document d;
  rapidjson::Value a(rapidjson::kArrayType);
  rapidjson::Document::AllocatorType &allocator = d.GetAllocator();
  for (int i = 0; i <= 255; i++)
    a.PushBack(histogram[i], allocator);

  rapidjson::StringBuffer buffer;
  buffer.Clear();
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  a.Accept(writer);

  auto res = string(buffer.GetString());
  response.setContentType("application/json");
  auto &ostr = response.send();
  ostr << res;
};