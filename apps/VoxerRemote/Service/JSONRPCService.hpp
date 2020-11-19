#pragma once
#include "Service/AbstractService.hpp"
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>
#include <stdexcept>

namespace voxer::remote {

class JSONRPCInvalidParamsError : public std::runtime_error {
public:
  JSONRPCInvalidParamsError() : runtime_error("Invalid Params") {}
};

using RPCHandler = std::function<rapidjson::Value(
    const std::vector<rapidjson::Value> &params)>;

template <class Param>
Param rpc_helper(const std::vector<rapidjson::Value> &params, size_t i) {
  Param param{};
  seria::deserialize(param, params[i]);
  return param;
};

template <typename ReturnType, typename... ParamTypes, std::size_t... index>
RPCHandler create_rpc_handler(std::function<ReturnType(ParamTypes...)> method,
                              std::index_sequence<index...>) {
  RPCHandler handler =
      [method](
          const std::vector<rapidjson::Value> &params) -> rapidjson::Value {
    size_t input_params_size = params.size();
    size_t params_size = sizeof...(ParamTypes);
    if (input_params_size != params_size) {
      throw JSONRPCInvalidParamsError();
    }

    auto result = method(
        rpc_helper<typename std::decay<ParamTypes>::type>(params, index)...);
    return seria::serialize(result);
  };
  return handler;
}

template <typename ReturnType, typename... ParamTypes>
RPCHandler GetHandler(std::function<ReturnType(ParamTypes...)> f) {
  return create_rpc_handle(f, std::index_sequence_for<ParamTypes...>{});;
}

template <typename ReturnType, typename... ParamTypes>
RPCHandler GetHandler(ReturnType (*f)(ParamTypes...)) {
  return GetHandler(std::function<ReturnType(ParamTypes...)>(f));
}

class RPCMethodsStore {
public:
  static auto get_instance() -> RPCMethodsStore *;

  auto invoke(const std::string &name, rapidjson::Value &params) -> rapidjson::Value;

  void resgister_method(std::string name, RPCHandler handler,
                        const std::vector<std::string> &params);

private:
  std::unordered_map<std::string, RPCHandler> m_methods;
  std::unordered_map<std::string, std::vector<std::string>> m_params;
  RPCMethodsStore() = default;
};

class JSONRPCService : public AbstractService {
public:
  enum class ErrorCode {
    ParseError = -32700,
    InvalidRequest = -32600,
    MethodNotFound = -32601,
    InvalidParams = -32602,
    InternalError = -32603,
    ServerError = -32000,
  };

  JSONRPCService();

  [[nodiscard]] std::string get_path() const noexcept override {
    return "/jsonrpc";
  }

  [[nodiscard]] Protocol get_protocol() const noexcept override {
    return AbstractService::Protocol::RPC;
  }

  void on_message(const char *message, uint32_t size,
                  const MessageCallback &callback) noexcept override;

  void register_method(std::string method, RPCHandler handler, const std::vector<std::string> &params_names = {});

  static auto ErrorMsg(ErrorCode code) -> const char *;

private:
  RPCMethodsStore *m_methods;

  static void on_error(ErrorCode error_code, rapidjson::Document &response);

  static void send(const rapidjson::Document &response,
                   const MessageCallback &callback);
};

} // namespace voxer::remote