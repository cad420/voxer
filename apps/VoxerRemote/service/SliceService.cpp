#include "service/SliceService.hpp"
#include <voxer/utils.hpp>

using namespace voxer;
using namespace std;

void SliceService::on_message(const char *message, uint32_t size) {
  m_document.Parse(message, size);

  if (!m_document.IsObject()) {
    throw JSON_error("root", "object");
  }

  auto params = m_document.GetObject();
  if (params.HasMember("dataset")) {
  }
}