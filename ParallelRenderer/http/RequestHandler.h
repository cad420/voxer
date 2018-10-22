#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/URI.h"
#include <string>
#include <vector>

class WebSocketRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  rapidjson::Document d;
  char buffer[1024 * 1024];
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response);
};

class ImageRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  std::vector<std::string> segments;
  Poco::URI uri;
  ImageRequestHandler(const Poco::URI &u) : uri(u) {
    uri.getPathSegments(this->segments);
  }
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response);
};

class JSONRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  Poco::URI uri;
  std::vector<std::string> segments;
  JSONRequestHandler(const Poco::URI &u) : uri(u) {
    uri.getPathSegments(this->segments);
  }
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response);
};
