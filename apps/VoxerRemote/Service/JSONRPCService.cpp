#include "JSONRPCService.hpp"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace voxer::remote {

auto JSONRPCService::ErrorMsg(ErrorCode code) -> const char * {
  static std::unordered_map<JSONRPCService::ErrorCode, const char *>
      message_map = {
          {ErrorCode::ParseError, "Parse error"},
          {ErrorCode::InvalidRequest, "Invalid Request"},
          {ErrorCode::MethodNotFound, "Method not found"},
          {ErrorCode::InvalidParams, "Invalid params"},
          {ErrorCode::InternalError, "Internal error"},
          {ErrorCode::ServerError, "Server error"},
      };

  return message_map[code];
}

void JSONRPCService::register_method(std::string method, RPCHandler handler, const std::vector<std::string> &params_names) {
  m_methods->resgister_method(std::move(method), std::move(handler), params_names);
}

void JSONRPCService::on_message(
    const char *message, uint32_t size,
    const voxer::remote::MessageCallback &callback) noexcept {
  // see https://www.jsonrpc.org/specification

  rapidjson::Document request{};
  request.Parse(message, size);

  rapidjson::Document response(rapidjson::kObjectType);
  auto &allocator = response.GetAllocator();
  response.AddMember("jsonrpc", "2.0", allocator);

  if (request.HasParseError()) {
    on_error(ErrorCode::ParseError, response);
    send(response, callback);
    return;
  }

  if (!request.IsObject() || !request.HasMember("method") ||
      !request.HasMember("id")) {
    on_error(ErrorCode::InvalidRequest, response);
    send(response, callback);
    return;
  }

  auto &request_id = request["id"];
  if (request_id.IsObject() || request.IsArray() || request.IsFloat() ||
      request.IsBool()) {
    on_error(ErrorCode::InvalidRequest, response);
    send(response, callback);
    return;
  }

  auto &request_method = request["method"];
  if (!request_method.IsString()) {
    on_error(ErrorCode::InvalidRequest, response);
    send(response, callback);
    return;
  }

  try {
    rapidjson::Value params{};
    if (request.HasMember("params")) {
      params = request["params"];
    }

    auto result = m_methods->invoke(request_method.GetString(), params);
    response.AddMember("id", request_id, allocator);
    response.AddMember("result", result, allocator);
    send(response, callback);
  } catch (std::exception &error) {
    on_error(ErrorCode::ServerError, response);
    send(response, callback);
  }
}

void JSONRPCService::on_error(JSONRPCService::ErrorCode error_code,
                              rapidjson::Document &response) {
  rapidjson::Value null{};
  auto &allocator = response.GetAllocator();
  rapidjson::Value error;
  error.AddMember("code", error_code, allocator);
  error.AddMember("message", rapidjson::StringRef(ErrorMsg(error_code)),
                  allocator);

  response.AddMember("id", null, allocator);
  response.AddMember("error", error, allocator);
}

void JSONRPCService::send(const rapidjson::Document &response,
                          const MessageCallback &callback) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  response.Accept(writer);

  callback(reinterpret_cast<const uint8_t *>(buffer.GetString()),
           buffer.GetSize(), false);
}

JSONRPCService::JSONRPCService() {
  m_methods = RPCMethodsStore::get_instance();
}

auto RPCMethodsStore::get_instance() -> RPCMethodsStore * {
  static RPCMethodsStore store{};
  return &store;
}

void RPCMethodsStore::resgister_method(std::string name, RPCHandler handler,
                                       const std::vector<std::string> &params) {
  m_methods.emplace(std::move(name), std::move(handler));
  if (!params.empty()) {
    m_params.emplace(name, params);
  }
}

auto RPCMethodsStore::invoke(const std::string &name,
                             rapidjson::Value &json_params)
    -> rapidjson::Value {
  auto it = m_methods.find(name);
  if (it == m_methods.end()) {
    throw "TODO";
  }

  auto handler = it->second;
  std::vector<rapidjson::Value> params;
  if (json_params.IsArray()) {
    for (int i = 0; i < json_params.Size(); i++) {
      params.push_back(json_params[i]);
    }
  } else if (json_params.IsObject()) {
    auto params_map_it = m_params.find(name);
    if (params_map_it == m_params.end()) {
      throw "TODO";
    }

    auto &params_names = params_map_it->second;
    for (auto &param_name : params_names) {
      if (!json_params.HasMember(param_name.c_str())) {
        throw "TODO";
      }
      params.push_back(json_params[param_name.c_str()]);
    }
  } else {
    throw "TODO";
  }

  try {
    return handler(params);
  } catch (std::exception &error) {
    throw "TODO";
  }
}

} // namespace voxer::remote
