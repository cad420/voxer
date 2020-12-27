#pragma once
#include "RPC/Error.hpp"
#include "Service/AbstractService.hpp"
#include "Store/DatasetStore.hpp"
#include <mpack/mpack.h>
#include <seria/serialize/mpack.hpp>
#include <seria/deserialize/mpack.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace voxer::remote {

class RPCMethodsStore {
public:
  using RPCHandler = std::function<void(const std::vector<mpack_node_t> &params,
                                        mpack_writer_t *writer)>;

  void invoke(const std::string &method, const mpack_node_t &params,
              mpack_writer_t *writer);

  void register_method(std::string name, RPCHandler handler,
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
    return GetHandler(function);
  }

private:
  std::unordered_map<std::string, RPCHandler> m_methods;
  std::unordered_map<std::string, std::vector<std::string>> m_params_names;

  template <typename Param>
  static Param RPCParamsUnpackHelper(const std::vector<mpack_node_t> &params,
                                     size_t i) {
    Param param{};
    seria::deserialize(param, params[i]);
    return param;
  }

  template <typename ReturnType, typename... ParamTypes, std::size_t... index>
  static RPCHandler
  CreateRPCHandler(std::function<ReturnType(ParamTypes...)> fn,
                   std::index_sequence<index...>) {
    RPCHandler handler = [fn](const std::vector<mpack_node_t> &params,
                              mpack_writer_t *writer) {
      size_t input_params_size = params.size();
      size_t params_size = sizeof...(ParamTypes);
      if (input_params_size != params_size) {
        throw JSONRPCInvalidParamsError();
      }

      auto result =
          fn(RPCParamsUnpackHelper<typename std::decay<ParamTypes>::type>(
              params, index)...);
      mpack_write_cstr(writer, "result");
      seria::serialize(result, writer);
    };
    return handler;
  }
};

} // namespace voxer::remote
