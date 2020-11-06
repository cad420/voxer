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
  explicit RPCRequestHandler(std::unique_ptr<AbstractService> service);
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response) override;

private:
  std::unique_ptr<AbstractService> m_service;
};

class WebSocketRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  explicit WebSocketRequestHandler(std::unique_ptr<AbstractService> service);
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response) override;

private:
  std::unique_ptr<AbstractService> m_service;
};

class MyHTTPRequestHandlerFactory
    : public Poco::Net::HTTPRequestHandlerFactory {
public:
  Poco::Net::HTTPRequestHandler *
  createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;

  void register_service(
      const char *path,
      const std::function<AbstractService *()> &constructor) noexcept;

private:
  std::unordered_map<std::string, std::function<AbstractService *()>> services;
};

} // namespace voxer::remote