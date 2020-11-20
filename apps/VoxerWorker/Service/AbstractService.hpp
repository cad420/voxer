#pragma once
#include "utils.hpp"
#include <rapidjson/document.h>
#include <mutex>

namespace voxer::remote {

using MessageCallback = std::function<void(const uint8_t *, uint32_t, bool)>;

class AbstractService : public NoCopy {
public:
  enum struct Protocol { WebSocket, RPC };

  ~AbstractService() = default;

  virtual void on_message(const char *message, uint32_t size,
                          const MessageCallback &callback) noexcept = 0;

  [[nodiscard]] virtual auto extract(const char *message, uint32_t size)
      -> std::pair<std::string, rapidjson::Value>;

  [[nodiscard]] virtual auto get_path() const noexcept -> std::string = 0;

  [[nodiscard]] virtual auto get_protocol() const noexcept -> Protocol = 0;

protected:
  rapidjson::Document m_document;
  std::mutex m_mutex;
};

} // namespace voxer::remote
