#include "RPC/dataset.hpp"
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/NullStream.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <iostream>
#include <sstream>

namespace {

bool request(Poco::Net::HTTPClientSession &session,
             Poco::Net::HTTPRequest &request,
             Poco::Net::HTTPResponse &response) {
  session.sendRequest(request);
  std::istream &rs = session.receiveResponse(response);
  auto status = response.getStatus();
  if (status != Poco::Net::HTTPResponse::HTTP_OK) {
    Poco::NullOutputStream null;
    Poco::StreamCopier::copyStream(rs, null);
    return false;
  }

  std::stringstream raw_json;
  Poco::StreamCopier::copyStream(rs, raw_json);
  auto s = raw_json.str();

  std::cout << s << std::endl;

  return true;
}

} // namespace

namespace voxer::remote {

auto query_dataset(const std::string &address, const std::string &id) -> std::promise<Dataset> {
  using Poco::Net::HTTPClientSession;
  using Poco::Net::HTTPRequest;
  using Poco::Net::HTTPResponse;

  Poco::URI uri{address};
  auto path = uri.getPathAndQuery();
  if (path.empty()) {
    path = "/";
  }

  HTTPClientSession session(uri.getHost(), uri.getPort());
  HTTPRequest req(HTTPRequest::HTTP_GET, path,
                  Poco::Net::HTTPMessage::HTTP_1_1);
  HTTPResponse res;
  request(session, req, res);

  return std::promise<Dataset>({});
}

} // namespace voxer::remote