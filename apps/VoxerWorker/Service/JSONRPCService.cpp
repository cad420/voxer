#include "JSONRPCService.hpp"
#include "JSONRPC/Error.hpp"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace voxer::remote {

void JSONRPCService::on_message(
    const char *message, uint32_t size,
    const voxer::remote::MessageCallback &callback) noexcept {
  // see https://www.jsonrpc.org/specification
  rapidjson::Document response(rapidjson::kObjectType);
  auto &allocator = response.GetAllocator();
  response.AddMember("jsonrpc", "2.0", allocator);

  try {
    rapidjson::Document request{};
    request.Parse(message, size);

    if (request.HasParseError()) {
      throw JSONRPCParseError();
    }

    if (!request.IsObject() || !request.HasMember("method") ||
        !request.HasMember("id")) {
      throw JSONRPCInvalidRequestError();
    }

    auto &request_id = request["id"];
    if (request_id.IsObject() || request.IsArray() || request.IsFloat() ||
        request.IsBool()) {
      throw JSONRPCInvalidRequestError();
    }

    auto &request_method = request["method"];
    if (!request_method.IsString()) {
      throw JSONRPCInvalidRequestError();
    }

    rapidjson::Value params{};
    if (request.HasMember("params")) {
      params = request["params"];
    }

    auto result = m_methods->invoke(request_method.GetString(), params);
    response.AddMember("id", request_id, allocator);
    response.AddMember("result", result, allocator);
  } catch (JSONRPCError &error) {
    on_error(error, response);
  } catch (std::exception &error) {
    on_error(JSONRPCServerError(error.what()), response);
  }

  send(response, callback);
}

void JSONRPCService::on_error(const JSONRPCError &error,
                              rapidjson::Document &response) {
  rapidjson::Value null{};
  auto &allocator = response.GetAllocator();
  rapidjson::Value response_error(rapidjson::kObjectType);
  response_error.AddMember("code", error.code(), allocator);
  rapidjson::Value msg(rapidjson::kStringType);
  msg.SetString(error.what(), std::strlen(error.what()), allocator);
  response_error.AddMember("message", msg, allocator);

  response.AddMember("id", null, allocator);
  response.AddMember("error", response_error, allocator);
}

void JSONRPCService::send(const rapidjson::Document &response,
                          const MessageCallback &callback) {
  rapidjson::StringBuffer buffer;
  buffer.Clear();
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  response.Accept(writer);

  callback(reinterpret_cast<const uint8_t *>(buffer.GetString()),
           buffer.GetSize(), false);
}

JSONRPCService::JSONRPCService() {
  m_methods = RPCMethodsStore::get_instance();
}

} // namespace voxer::remote
