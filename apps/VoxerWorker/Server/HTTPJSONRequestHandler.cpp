#include "Server/HTTPJSONRequestHandler.hpp"
#include <Poco/Net/NetException.h>
#include <Poco/Util/Application.h>

using Poco::Util::Application;

namespace voxer::remote {

void HTTPJSONRequestHandler::handleRequest(
    Poco::Net::HTTPServerRequest &request,
    Poco::Net::HTTPServerResponse &response) {
  std::istream &istr = request.stream();
  int len = request.getContentLength();

  std::unique_ptr<char[]> buffer;
  if (len > 0) {
    buffer.reset(new char[len]);
  } else {
    len = 0;
  }

  istr.read(buffer.get(), len);
  response.setKeepAlive(false);
  response.setContentType("application/json");
  m_service->on_message(
      buffer.get(), len,
      [&response](const uint8_t *message, uint32_t size, bool is_binary) {
        response.setContentLength(size);
        std::ostream &ostr = response.send();
        ostr.write(reinterpret_cast<const char *>(message), size);
      });
}

HTTPJSONRequestHandler::HTTPJSONRequestHandler(
    std::unique_ptr<AbstractService> service) {
  assert(service != nullptr);
  m_service = move(service);
}

} // namespace voxer::remote