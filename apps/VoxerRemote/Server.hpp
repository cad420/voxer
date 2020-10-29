#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

namespace voxer::remote {

class MyHTTPRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response) override;
};

class MyHTTPRequestHandlerFactory
    : public Poco::Net::HTTPRequestHandlerFactory {
public:
  Poco::Net::HTTPRequestHandler *
  createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;
};

} // namespace voxer::remote