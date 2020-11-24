#include "Server/RequestHandlerFactory.hpp"
#include "Server/HTTPJSONRequestHandler.hpp"
#include "Server/WebSockeRequestHandler.hpp"
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/NetException.h>
#include <Poco/Util/Application.h>

using Poco::Util::Application;

namespace {

class DefaultRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response) override {
    auto &uri = request.getURI();
    response.setContentType("text/plain");
    if (uri != "/") {
      response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
      std::ostream &ostr = response.send();
      ostr << "Not Found.";
      return;
    }

    std::ostream &ostr = response.send();
    ostr << "Welcome to voxer.";
  }
};

} // namespace

namespace voxer::remote {

Poco::Net::HTTPRequestHandler *RequestHandlerFactory::createRequestHandler(
    const Poco::Net::HTTPServerRequest &request) {
  auto &uri = request.getURI();

  auto it = services.find(uri);
  if (it == services.end()) {
    return new DefaultRequestHandler();
  }

  const auto &construtor = it->second;
  auto service = std::unique_ptr<AbstractService>(construtor());
  if (service->get_protocol() == AbstractService::Protocol::RPC) {
    return new HTTPJSONRequestHandler(std::move(service));
  } else if (service->get_protocol() == AbstractService::Protocol::WebSocket) {
    return new WebSocketRequestHandler(std::move(service));
  }

  return new DefaultRequestHandler();
}

} // namespace voxer::remote