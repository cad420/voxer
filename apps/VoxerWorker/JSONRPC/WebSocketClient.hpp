#pragma once
#include <Poco/Net/WebSocket.h>
#include <cstdint>
#include <memory>

namespace voxer::remote {

class WebSocketClient {
public:
  using MessageCallback = std::function<void(uint8_t *message, uint32_t size, bool is_binary)>;

  WebSocketClient() = default;
  ~WebSocketClient() noexcept;

  void connect(const char *host, uint16_t port,
               const char *path);

  void send(uint8_t *message, uint32_t size, bool is_binary);

  void on_message(const MessageCallback &callback);

private:
  std::unique_ptr<Poco::Net::WebSocket> m_ws = nullptr;
  MessageCallback
      m_handle_message = nullptr;
};

} // namespace voxer::remote