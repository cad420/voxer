#pragma once
#include "Store/DatasetStore.hpp"
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <RPC/RPCMethodsStore.hpp>
#include <unordered_map>

namespace voxer::remote {

class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
  explicit RequestHandlerFactory(DatasetStore *);

  Poco::Net::HTTPRequestHandler *
  createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;

private:
  DatasetStore *m_dataset;
};

} // namespace voxer::remote