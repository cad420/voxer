#include "Service/SliceService.hpp"
#include "DataModel/Slice.hpp"
#include <fmt/format.h>
#include <seria/deserialize.hpp>

using namespace voxer;
using namespace std;

namespace voxer::remote {

void SliceService::on_message(const char *message, uint32_t size, const MessageCallback &callback) noexcept {
  assert(m_datasets != nullptr && callback != nullptr);

  try {
    m_document.Parse(message, size);

    if (!m_document.IsObject()) {
      throw std::runtime_error("root should be an object");
    }

    auto params = m_document.GetObject();
    if (!params.HasMember("type") || !params["type"].IsString()) {
      throw std::runtime_error("root.type should be a string");
    }

    std::string type = params["type"].GetString();
    if (type == "slice") {
      Slice slice{};
      seria::deserialize(slice, params);
      auto image = get_dataset_slice(slice.dataset, slice.axis, slice.index);
      callback(reinterpret_cast<const uint8_t *>(image.data.data()),
             image.data.size(), true);
    }
  } catch (exception &error) {
    auto error_msg = fmt::format(R"({{"error": "{}"}})", error.what());
    callback(reinterpret_cast<const uint8_t *>(error_msg.data()),
           error_msg.size(), false);
  }
}

auto SliceService::get_dataset_slice(const DatasetId &dataset_id,
                                     StructuredGrid::Axis axis,
                                     uint32_t index) const -> Image {
  auto dataset = m_datasets->get(dataset_id);
  auto image = dataset->get_slice(axis, index);
  auto jpeg = Image::encode(image, Image::Format::JPEG);
  return jpeg;
}

} // namespace voxer::remote