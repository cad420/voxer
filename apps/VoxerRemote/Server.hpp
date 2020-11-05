#pragma once
#include "Service/AbstractService.hpp"
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <vector>

namespace voxer::remote {

class DefaultRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response) override;
};

class RPCRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  explicit RPCRequestHandler(AbstractService *service);
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response) override;

private:
  AbstractService *m_service;
};

class WebSocketRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  explicit WebSocketRequestHandler(AbstractService *service);
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response) override;

private:
  AbstractService *m_service;
};

class MyHTTPRequestHandlerFactory
    : public Poco::Net::HTTPRequestHandlerFactory {
public:
  Poco::Net::HTTPRequestHandler *
  createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;

  void add_service(std::unique_ptr<AbstractService> &&service) noexcept;

private:
  std::vector<std::unique_ptr<AbstractService>> services;
};

} // namespace voxer::remote