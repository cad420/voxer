#pragma once
#include "Service/AbstractService.hpp"
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Store/DatasetStore.hpp>

namespace voxer::remote {

class WebSocketRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  explicit WebSocketRequestHandler(std::unique_ptr<AbstractService> service);
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response) override;

private:
  std::unique_ptr<AbstractService> m_service;
};

} // namespace voxer::remote