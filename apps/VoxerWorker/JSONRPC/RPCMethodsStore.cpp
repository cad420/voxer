#include "RPCMethodsStore.hpp"
#include <spdlog/spdlog.h>

namespace voxer::remote {

void RPCMethodsStore::resgister_method(std::string name, RPCHandler handler,
                                       const std::vector<std::string> &params) {
  m_methods.emplace(name, std::move(handler));
  if (!params.empty()) {
    m_params_names.emplace(std::move(name), params);
  }
}

rapidjson::Document RPCMethodsStore::invoke(const std::string &name,
                                            rapidjson::Value &json_params) {
  auto it = m_methods.find(name);
  if (it == m_methods.end()) {
    throw JSONRPCMethodNotFoundError();
  }

  auto handler = it->second;
  std::vector<rapidjson::Value> params;
  if (json_params.IsArray()) {
    params.resize(json_params.Size());
    for (int i = 0; i < json_params.Size(); i++) {
      params[i] = json_params[i];
    }
  } else if (json_params.IsObject()) {
    auto params_map_it = m_params_names.find(name);
    if (params_map_it == m_params_names.end()) {
      throw JSONRPCInvalidParamsError();
    }

    auto &param_names = params_map_it->second;
    params.resize(param_names.size());
    for (size_t i = 0; i < param_names.size(); i++) {
      auto &param_name = param_names[i];
      if (!json_params.HasMember(param_name.c_str())) {
        throw JSONRPCInvalidParamsError();
      }
      params[i] = json_params[param_name.c_str()];
    }
  } else {
    throw JSONRPCInvalidParamsError();
  }

  spdlog::info("invoke RPC method {}", name.c_str());
  return handler(params);
}

auto RPCMethodsStore::get_instance() -> RPCMethodsStore * {
  static RPCMethodsStore store{};
  return &store;
}

} // namespace voxer::remote