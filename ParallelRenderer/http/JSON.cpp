#include "ParallelRenderer/ConfigManager.h"
#include "ParallelRenderer/Encoder.h"
#include "ParallelRenderer/Renderer.h"
#include "RequestHandler.h"
#include <map>

using namespace std;
using ospcommon::vec2i;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

extern ConfigManager configs;
extern Renderer renderer;
extern Encoder encoder;

void JSONRequestHandler::handleRequest(HTTPServerRequest &request,
                                       HTTPServerResponse &response) {
  response.setChunkedTransferEncoding(true);
  response.add("Access-Control-Allow-Origin", "*");
  response.add("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  response.add("Access-Control-Allow-Headers", "content-type");

  auto id = segments[0];
  auto &config = configs.get(id);
  response.setContentType("text/html");
  response.setStatus(HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
  response.setReason("Not Found");
  auto &ostr = response.send();
  ostr << "404, Not Found";
};