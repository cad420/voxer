#include "Service/SliceService.hpp"
#include <voxer/utils.hpp>
#include <voxer/Slice.hpp>
#include <seria/deserialize.hpp>

using namespace voxer;
using namespace std;

void SliceService::on_message(const char *message, uint32_t size) {
  m_document.Parse(message, size);

  if (!m_document.IsObject()) {
    throw std::runtime_error("root should be an object");
    return;
  }

  auto params = m_document.GetObject();
  if (!params.HasMember("type") || !params["type"].IsString()) {
    throw std::runtime_error("root.type should be a string");
  }

  std::string type = params["type"].GetString();
  if (type == "slice") {
    SliceInfo slice{};
    seria::deserialize(slice, params);
    auto image = get_dataset_slice(slice.dataset_id, slice.axis, slice.index);
    cb(reinterpret_cast<const uint8_t *>(image.data.data()), image.data.size(),
       true);
    auto it = annotation_store.find(slice);
    if (it != annotation_store.end()) {
      auto json = it->second.serialize();
      auto result =
          std::string(R"({"type":"annotation","data":)") + json + R"(})";
      cb(reinterpret_cast<const uint8_t *>(result.data()), result.size(),
         false);
    }
    return;
  }
}