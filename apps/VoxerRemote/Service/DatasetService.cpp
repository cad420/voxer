#include "DatasetService.hpp"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <seria/serialize.hpp>
#include <voxer/Filters/histogram.hpp>

using namespace std;

namespace voxer::remote {

void DatasetService::on_message(const char *message, uint32_t size) {
  if (m_datasets == nullptr || m_send == nullptr) {
    return;
  }

  load_dataset(message, size);
}

void DatasetService::load_dataset(const char *msg, uint32_t size) {
  if (m_datasets == nullptr) {
    throw runtime_error("No dataset store");
  }

  if (m_send == nullptr) {
    throw runtime_error("No send function");
  }

  rapidjson::StringBuffer buffer;
  buffer.Clear();
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  m_datasets->load_from_json(msg, size);
  auto &items = m_datasets->get();
  for (auto &item : items) {
    auto histogram = voxer::calculate_histogram(*(item.second));
    auto serialized = seria::serialize(histogram);
    serialized.Accept(writer);

    m_send(reinterpret_cast<const uint8_t *>(buffer.GetString()),
           buffer.GetSize(), false);
  }
}

} // namespace voxer::remote