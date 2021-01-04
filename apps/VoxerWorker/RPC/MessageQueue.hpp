#pragma once
#include "RPC/Service.hpp"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace voxer::remote {

class MessageQueue {
public:
  struct Task {
    // TODO: use memory pool
    std::vector<uint8_t> message = {};
    Service::Callback on_processed = nullptr;
  };

  static MessageQueue &get_instance();

  void add_message(const uint8_t *msg, uint32_t size,
                   const Service::Callback &callback) noexcept;

private:
  MessageQueue();

  ~MessageQueue();

  void process();

  std::atomic<bool> m_running = false;
  std::mutex m_mutex;
  std::condition_variable m_cv;
  std::vector<std::thread> m_threads{};
  std::queue<Task> m_tasks{};
};

} // namespace voxer::remote
