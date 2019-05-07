#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/URI.h"
#include <string>
#include <vector>

enum class DataType { rendering, histogram, scatter, slice, unsupported, text };

class WebSocketRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  rapidjson::Document d;
  char buffer[1024 * 1024 * 1024];
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response);
};

class ImageRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  std::vector<std::string> segments;
  Poco::URI uri;
  DataType type;
  ImageRequestHandler(const Poco::URI &u, const DataType &t) : uri(u), type(t) {
    uri.getPathSegments(this->segments);
  }
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response);
};

class JSONRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  Poco::URI uri;
  DataType type;
  std::vector<std::string> segments;
  JSONRequestHandler(const Poco::URI &u, const DataType &t) : uri(u), type(t) {
    uri.getPathSegments(this->segments);
  }
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response);
};

class DefaultRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  DataType type;
  DefaultRequestHandler(): type(DataType::text) {}
  DefaultRequestHandler(DataType t): type(t) {}
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response) {
    response.setChunkedTransferEncoding(true);
    response.add("Access-Control-Allow-Origin", "*");
    response.add("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    response.add("Access-Control-Allow-Headers", "content-type");

    response.setContentType("text/html");
    auto &ostr = response.send();
    if (type == DataType::unsupported) {
      ostr << "Unsupported data type!";
    } else {
      ostr << "Websocket Server has been started!";
    }
    return;
  }
};