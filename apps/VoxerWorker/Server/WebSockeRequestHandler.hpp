#pragma once
#include "RPC/Service.hpp"
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Store/DatasetStore.hpp>

namespace voxer::remote {

class WebSocketRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  explicit WebSocketRequestHandler(DatasetStore *m_dataset);
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response) override;

private:
  DatasetStore *m_datasets = nullptr;
  std::unique_ptr<Service> m_service = nullptr;
};

} // namespace voxer::remote