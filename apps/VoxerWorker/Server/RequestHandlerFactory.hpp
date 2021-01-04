#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>

namespace voxer::remote {

class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
  Poco::Net::HTTPRequestHandler *
  createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;
};

} // namespace voxer::remote