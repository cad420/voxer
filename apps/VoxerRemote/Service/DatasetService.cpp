#include "DatasetService.hpp"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <seria/serialize.hpp>
#include <seria/utils.hpp>
#include <voxer/Filters/histogram.hpp>
#include <iostream>

using namespace std;

namespace {

struct LoadDatasetResponse {
  uint32_t id = 0;
  std::array<uint32_t, 3> dimensions {};
  std::vector<uint32_t> histogram {};
};

} // namespace

namespace seria {

template <> inline auto registerObject<LoadDatasetResponse>() {
  return std::make_tuple(
      member("id", &LoadDatasetResponse::id),
      member("dimensions", &LoadDatasetResponse::dimensions),
                         member("histogram", &LoadDatasetResponse::histogram));
}

} // namespace seria

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

  m_datasets->load_from_json(msg, size);

  LoadDatasetResponse res;
  const auto &items = m_datasets->get();
  for (auto &item : items) {
    res.id = item.first;
    res.histogram = voxer::calculate_histogram(*item.second);
    res.dimensions = item.second->info.dimensions;
    auto serialized = seria::serialize(res);

    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    serialized.Accept(writer);

    m_send(reinterpret_cast<const uint8_t *>(buffer.GetString()),
           buffer.GetSize(), false);
  }
}

} // namespace voxer::remote