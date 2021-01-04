#include "RPC/MessageQueue.hpp"
#include "RPC/Service.hpp"

namespace voxer::remote {

MessageQueue::MessageQueue() {
  m_running = true;
  for (int i = 0; i < std::thread::hardware_concurrency(); i++) {
    m_threads.emplace_back(&MessageQueue::process, this);
  }
}

MessageQueue::~MessageQueue() {
  if (!m_running) {
    return;
  }

  m_running = false;
  m_cv.notify_all();

  for (auto &thread : m_threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

MessageQueue &MessageQueue::get_instance() {
  static MessageQueue queue{};

  return queue;
}

void MessageQueue::add_message(const uint8_t *msg, uint32_t size,
                               const Service::Callback &callback) noexcept {
  std::unique_lock<std::mutex> lock(m_mutex);
  MessageQueue::Task task{std::vector<uint8_t>(msg, msg + size), callback};
  m_tasks.emplace(std::move(task));
  m_cv.notify_one();
}

void MessageQueue::process() {
  Service service{&DatasetStore::default_instance()};

  while (m_running) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_tasks.empty()) {
      auto task = std::move(m_tasks.front());
      m_tasks.pop();
      lock.unlock();

      service.on_message(task.message.data(), task.message.size(),
                         task.on_processed);
    } else if (m_running) {
      m_cv.wait(lock);
      lock.unlock();
    }
  }
}

} // namespace voxer::remote
