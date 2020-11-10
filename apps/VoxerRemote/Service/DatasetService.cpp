#include "DatasetService.hpp"
#include <fmt/format.h>
#include <iostream>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <seria/deserialize.hpp>
#include <seria/object.hpp>
#include <seria/serialize.hpp>
#include <voxer/Filters/histogram.hpp>

using namespace std;

namespace seria {

template <> inline auto register_object<voxer::remote::LoadDataSetParams>() {
  using Object = voxer::remote::LoadDataSetParams;
  return std::make_tuple(member("id", &Object::id));
}

template <> inline auto register_object<voxer::remote::LoadDatasetResponse>() {
  using Object = voxer::remote::LoadDatasetResponse;
  return std::make_tuple(
      member("id", &Object::id), member("dimensions", &Object::dimensions),
      member("histogram", &Object::histogram), member("range", &Object::range));
}

} // namespace seria

namespace voxer::remote {

void DatasetService::on_message(const char *message, uint32_t size,
                                const MessageCallback &callback) noexcept {
  assert(m_datasets != nullptr && callback != nullptr);

  if (m_datasets == nullptr) {
    return;
  }

  try {
    auto [function_name, json] = extract(message, size);

    if (function_name == "query_dataset") {
      LoadDataSetParams params{};
      seria::deserialize(params, json);

      LoadDatasetResponse result = query_dataset(params);

      auto serialized = seria::serialize(result);
      rapidjson::StringBuffer buffer;
      buffer.Clear();
      rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
      serialized.Accept(writer);

      callback(reinterpret_cast<const uint8_t *>(buffer.GetString()),
               buffer.GetSize(), false);
      return;
    }

    auto error_msg = "unknown function: " + function_name;
    callback(reinterpret_cast<const uint8_t *>(error_msg.c_str()),
             error_msg.size(), false);
  } catch (exception &error) {
    auto error_msg = fmt::format(R"({{"error": "{}"}})", error.what());
    callback(reinterpret_cast<const uint8_t *>(error_msg.data()),
             error_msg.size(), false);
  }
}

auto DatasetService::query_dataset(const LoadDataSetParams &params) const
    -> LoadDatasetResponse {
  auto dataset = m_datasets->get(params.id);

  LoadDatasetResponse result;
  result.id = params.id;
  result.histogram = voxer::calculate_histogram(*dataset);
  result.dimensions = dataset->info.dimensions;
  result.range = dataset->original_range;

  return result;
}

} // namespace voxer::remote