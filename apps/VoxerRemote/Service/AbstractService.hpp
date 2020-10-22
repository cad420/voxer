#pragma once
#include "utils.hpp"
#include <rapidjson/document.h>

namespace voxer::remote {

class AbstractService : public NoCopy {
public:
  enum struct Protocol { WebSocket, HTTP };

  ~AbstractService() = default;

  virtual void on_message(const char *message, uint32_t size) = 0;

  [[nodiscard]] virtual auto extract(const char *message, uint32_t size)
      -> std::pair<std::string, rapidjson::Value>;

  [[nodiscard]] virtual auto get_path() const noexcept -> std::string = 0;

  [[nodiscard]] virtual auto get_protocol() const noexcept -> Protocol = 0;

  std::function<void(const uint8_t *message, uint32_t size, bool is_binary)>
      m_send = nullptr;

protected:
  rapidjson::Document m_document;
};

} // namespace voxer::remote
