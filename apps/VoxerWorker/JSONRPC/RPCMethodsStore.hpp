#pragma once
#include "JSONRPC/Error.hpp"
#include "Service/AbstractService.hpp"
#include <Store/DatasetStore.hpp>
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>
#include <stdexcept>

namespace voxer::remote {

using RPCHandler = std::function<rapidjson::Document(
    const std::vector<rapidjson::Value> &params)>;

class RPCMethodsStore {
public:
  static auto get_instance() -> RPCMethodsStore *;

  auto invoke(const std::string &name, rapidjson::Value &params)
      -> rapidjson::Document;

  void resgister_method(std::string name, RPCHandler handler,
                        const std::vector<std::string> &params = {});

  template <typename ReturnType, typename... ParamTypes>
  static RPCHandler GetHandler(std::function<ReturnType(ParamTypes...)> f) {
    return CreateRPCHandler(f, std::index_sequence_for<ParamTypes...>{});
  }

  template <typename ReturnType, typename... ParamTypes>
  static RPCHandler GetHandler(ReturnType (*f)(ParamTypes...)) {
    return GetHandler(std::function<ReturnType(ParamTypes...)>(f));
  }

  template <typename T, typename ReturnType, typename... ParamTypes>
  static RPCHandler GetHandler(ReturnType (T::*method)(ParamTypes...),
                               T &instance) {
    std::function<ReturnType(ParamTypes...)> function =
        [&instance, method](ParamTypes &&... params) -> ReturnType {
      return (instance.*method)(std::forward<ParamTypes>(params)...);
    };
    return RPCHandler(function);
  }

private:
  std::unordered_map<std::string, RPCHandler> m_methods;
  std::unordered_map<std::string, std::vector<std::string>> m_params_names;

  RPCMethodsStore() = default;

  template <typename Param>
  static Param
  RPCParamsUnpackHelper(const std::vector<rapidjson::Value> &params,
                           size_t i) {
    Param param{};
    seria::deserialize(param, params[i]);
    return param;
  }

  template <typename ReturnType, typename... ParamTypes, std::size_t... index>
  static RPCHandler
  CreateRPCHandler(std::function<ReturnType(ParamTypes...)> method,
                     std::index_sequence<index...>) {
    RPCHandler handler = [method](const std::vector<rapidjson::Value> &params)
        -> rapidjson::Document {
      size_t input_params_size = params.size();
      size_t params_size = sizeof...(ParamTypes);
      if (input_params_size != params_size) {
        throw JSONRPCInvalidParamsError();
      }

      auto result = method(RPCParamsUnpackHelper<typename std::decay<ParamTypes>::type>(
              params, index)...);
      auto json_result = seria::serialize(result);
      return json_result;
    };
    return handler;
  }
};

} // namespace voxer::remote
