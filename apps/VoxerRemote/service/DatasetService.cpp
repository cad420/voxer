#include "DatasetService.hpp"

using namespace std;

void DatasetService::on_message(const char *message, uint32_t size) {
  if (m_datasets == nullptr || m_send == nullptr) {
    return;
  }

  load_dataset(message, size);
}

void DatasetService::load_dataset(const char *json, uint32_t size) {
  if (m_datasets == nullptr) {
    throw runtime_error("No dataset store");
  }

  if (m_send == nullptr) {
    throw runtime_error("No send function");
  }

  m_datasets->load_from_json(json, size);
  auto &items = m_datasets->get();
  for (auto &item : items) {
    auto msg = item.serialize();
    m_send(reinterpret_cast<const uint8_t *>(msg.data()), msg.size(), false);
  }
}
