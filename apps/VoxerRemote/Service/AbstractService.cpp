#include "AbstractService.hpp"

using namespace std;

namespace voxer::remote {

auto AbstractService::extract(const char *message, uint32_t size)
    -> std::pair<std::string, rapidjson::Value> {
  m_document.Parse(message, size);

  if (!m_document.IsObject()) {
    throw std::runtime_error("message should be an JSON object");
  }

  auto request = m_document.GetObject();

  auto it = request.FindMember("function");
  if (it == request.end() || !it->value.IsString()) {
    throw std::runtime_error(
        "message object should have member `function` of type string");
  }
  std::string function_name = it->value.GetString();

  it = request.FindMember("params");
  if (it == request.end() || !it->value.IsObject()) {
    throw std::runtime_error("message object should have member `params`");
  }
  auto params = it->value.GetObject();

  return std::make_pair(std::move(function_name), std::move(params));
}

} // namespace voxer::remote