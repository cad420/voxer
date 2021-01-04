#pragma once
#include "RPC/MessageQueue.hpp"
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

namespace voxer::remote {

class WebSocketRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  WebSocketRequestHandler();

  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response) override;

private:
  MessageQueue *m_queue = nullptr;
};

} // namespace voxer::remote