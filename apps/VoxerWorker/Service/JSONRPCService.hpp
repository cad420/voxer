#pragma once
#include "JSONRPC/Error.hpp"
#include "JSONRPC/RPCMethodsStore.hpp"
#include "Service/AbstractService.hpp"
#include <Store/DatasetStore.hpp>
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>
#include <stdexcept>

namespace voxer::remote {

class JSONRPCService : public AbstractService {
public:
  JSONRPCService();

  [[nodiscard]] Protocol get_protocol() const noexcept override {
    return AbstractService::Protocol::RPC;
  }

  void on_message(const char *message, uint32_t size,
                  const MessageCallback &callback) noexcept override;

  DatasetStore *m_datasets = nullptr;

private:
  RPCMethodsStore *m_methods;

  static void on_error(const JSONRPCError &error,
                       rapidjson::Document &response);

  static void send(const rapidjson::Document &response,
                   const MessageCallback &callback);
};

} // namespace voxer::remote