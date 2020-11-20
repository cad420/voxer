#include "ManagerAPI/dataset.hpp"
#include "DataModel/StructuredGrid.hpp"
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/NullStream.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <iostream>
#include <seria/deserialize.hpp>
#include <seria/object.hpp>
#include <sstream>

namespace {

struct DatasetResponse {
  int code = 0;
  voxer::remote::Dataset data;
};

auto request(Poco::Net::HTTPClientSession &session,
             Poco::Net::HTTPRequest &request, Poco::Net::HTTPResponse &response)
    -> voxer::remote::Dataset {
  session.sendRequest(request);
  std::istream &rs = session.receiveResponse(response);
  auto status = response.getStatus();
  if (status != Poco::Net::HTTPResponse::HTTP_OK) {
    Poco::NullOutputStream null;
    Poco::StreamCopier::copyStream(rs, null);
    throw std::runtime_error("dataset request failed");
  }

  std::stringstream raw_json;
  Poco::StreamCopier::copyStream(rs, raw_json);
  auto s = raw_json.str();

  rapidjson::Document document;
  document.Parse(s.c_str(), s.size());
  DatasetResponse res_data{};
  seria::deserialize(res_data, document.GetObject());

  return res_data.data;
}

} // namespace

namespace seria {

template <> inline auto register_object<DatasetResponse>() {
  return std::make_tuple(member("code", &DatasetResponse::code),
                         member("data", &DatasetResponse::data));
}

} // namespace seria

namespace voxer::remote {

auto get_dataset_info(const std::string &address, const std::string &id)
    -> Dataset {
  using Poco::Net::HTTPClientSession;
  using Poco::Net::HTTPRequest;
  using Poco::Net::HTTPResponse;

  Poco::URI uri{"http://" + address + "/datasets/" + id};
  auto path = uri.getPathAndQuery();
  if (path.empty()) {
    path = "/";
  }

  HTTPClientSession session(uri.getHost(), uri.getPort());
  HTTPRequest req(HTTPRequest::HTTP_GET, path,
                  Poco::Net::HTTPMessage::HTTP_1_1);
  HTTPResponse res;

  return request(session, req, res);
}

} // namespace voxer::remote