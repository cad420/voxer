#include "Server/RequestHandlerFactory.hpp"
#include "Server/WebSockeRequestHandler.hpp"
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/NetException.h>

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
  if (request.getURI() != "/rpc") {
    return new DefaultRequestHandler();
  }

  return new WebSocketRequestHandler();
}


} // namespace voxer::remote