#pragma once
#include "DataModel/DatasetLoadInfo.hpp"
#include "JSONRPC/WebSocketClient.hpp"
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/NullStream.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <fmt/core.h>
#include <seria/deserialize.hpp>
#include <seria/object.hpp>
#include <sstream>
#include <string>
#include <thread>

namespace voxer::remote {

class VolumeRenderingService;

class ManagerAPIClient {
public:
  explicit ManagerAPIClient();
  explicit ManagerAPIClient(std::string address);
  ~ManagerAPIClient();

  ManagerAPIClient(ManagerAPIClient &&another) noexcept ;
  ManagerAPIClient & operator=(ManagerAPIClient &&another) noexcept;

  DatasetLoadInfo get_dataset_info(const std::string &id);

  const char *get_address() const noexcept { return m_address.c_str(); }

  std::unique_ptr<VolumeRenderingService> m_service;
private:
  std::string m_address;
  Poco::URI m_uri;
  std::unique_ptr<WebSocketClient> m_ws = nullptr;
  std::thread m_thread {};

  template <typename ResultType> ResultType request(const std::string &path) {
    using Poco::Net::HTTPClientSession;
    using Poco::Net::HTTPRequest;
    using Poco::Net::HTTPResponse;

    HTTPClientSession session(m_uri.getHost(), m_uri.getPort());
    HTTPRequest req(HTTPRequest::HTTP_GET, path,
                    Poco::Net::HTTPMessage::HTTP_1_1);
    HTTPResponse res;

    session.sendRequest(req);
    std::istream &rs = session.receiveResponse(res);
    auto status = res.getStatus();
    if (status != Poco::Net::HTTPResponse::HTTP_OK) {
      Poco::NullOutputStream null;
      Poco::StreamCopier::copyStream(rs, null);
      throw std::runtime_error(
          "Request failed, status: " +
          Poco::Net::HTTPResponse::getReasonForStatus(status));
    }

    std::stringstream raw_json;
    Poco::StreamCopier::copyStream(rs, raw_json);
    auto s = raw_json.str();

    rapidjson::Document document;
    document.Parse(s.c_str(), s.size());
    if (document.HasParseError()) {
      throw std::runtime_error("invalid response, should be a json");
    }

    if (!document.IsObject()) {
      throw std::runtime_error("invalid response, should be object");
    }

    auto value = document.GetObject();
    if (!value.HasMember("code") || !value["code"].IsInt()) {
      throw std::runtime_error("invalid response, response.code be an integer");
    }

    auto code = value["code"].GetInt();

    switch (code) {
    case 200: {
      ResultType data{};
      seria::deserialize(data, value["data"]);
      return data;
    }
    default: {
      if (value.HasMember("data") && value["data"].IsString()) {
        throw std::runtime_error(
            fmt::format("Error({}): {}", code, value["data"].GetString()));
      }
      throw std::runtime_error(fmt::format("Error({})", code));
    }
    }
  }
};

} // namespace voxer::remote