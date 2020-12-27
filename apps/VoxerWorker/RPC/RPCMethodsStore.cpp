#include "RPCMethodsStore.hpp"
#include <spdlog/spdlog.h>

namespace voxer::remote {

void RPCMethodsStore::register_method(std::string name, RPCHandler handler,
                                      const std::vector<std::string> &params) {
  m_methods.emplace(name, std::move(handler));
  if (!params.empty()) {
    m_params_names.emplace(std::move(name), params);
  }
}

void RPCMethodsStore::invoke(const std::string &method,
                             const mpack_node_t &node, mpack_writer_t *writer) {
  auto it = m_methods.find(method);
  if (it == m_methods.end()) {
    throw JSONRPCMethodNotFoundError();
  }

  auto handler = it->second;
  std::vector<mpack_node_t> params;
  if (node.data->type == mpack_type_array) {
    auto params_size = mpack_node_array_length(node);
    params.resize(params_size);
    for (int i = 0; i < params_size; i++) {
      params[i] = mpack_node_array_at(node, i);
    }
  } else if (node.data->type == mpack_type_map) {
    auto params_map_it = m_params_names.find(method);
    if (params_map_it == m_params_names.end()) {
      throw JSONRPCInvalidParamsError();
    }

    auto &param_names = params_map_it->second;
    params.resize(param_names.size());
    for (size_t i = 0; i < param_names.size(); i++) {
      auto &param_name = param_names[i];
      if (!mpack_node_map_contains_cstr(node, param_name.c_str())) {
        throw JSONRPCInvalidParamsError();
      }
      params[i] = mpack_node_map_cstr(node, param_name.c_str());
    }
  } else {
    throw JSONRPCInvalidParamsError();
  }

  spdlog::info("Invoke RPC method {}", method.c_str());

  handler(params, writer);
}

} // namespace voxer::remote