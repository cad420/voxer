#include "Server.hpp"
#include <Poco/Util/Application.h>

using Poco::Util::Application;

namespace voxer::remote {

void MyHTTPRequestHandler::handleRequest(
    Poco::Net::HTTPServerRequest &request,
    Poco::Net::HTTPServerResponse &response) {
  Application &app = Application::instance();
  app.logger().information("Request from %s",
                           request.clientAddress().toString());

  response.setChunkedTransferEncoding(true);
  response.setContentType("text/html");

  std::ostream &ostr = response.send();
  ostr << "<html><head><title>Test</title><body>test</body></html>";
}

Poco::Net::HTTPRequestHandler *
MyHTTPRequestHandlerFactory::createRequestHandler(
    const Poco::Net::HTTPServerRequest &request) {
  auto &uri = request.getURI();

  if (uri == "/") {
    return new MyHTTPRequestHandler();
  } else {
    return nullptr;
  }
}

} // namespace voxer::remote