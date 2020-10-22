#include "AnnotationService.hpp"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <seria/deserialize.hpp>
#include <seria/object.hpp>
#include <seria/serialize.hpp>
#include <voxer/Filters/AnnotationLevelset.hpp>

namespace seria {

template <>
inline auto register_object<voxer::remote::AnnotationLevelSetParams>() {
  using Object = voxer::remote::AnnotationLevelSetParams;
  return std::make_tuple(member("dataset", &Object::dataset),
                         member("axis", &Object::axis),
                         member("index", &Object::index),
                         member("annotations", &Object::annotations));
}

} // namespace seria

namespace voxer::remote {

void AnnotationService::on_message(const char *message, uint32_t size) {
  if (m_send == nullptr || m_datasets == nullptr) {
    throw std::runtime_error("datasets and send function not set.");
  }

  auto extracted = extract(message, size);
  auto &function_name = extracted.first;
  auto &json = extracted.second;

  if (function_name == "apply_level_set") {
    AnnotationLevelSetParams params{};
    seria::deserialize(params, json);

    auto axis = StructuredGrid::get_axis(params.axis.c_str());
    auto result =
        apply_level_set(params.annotations, params.dataset, axis, params.index);

    auto serialized = seria::serialize(result);
    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    serialized.Accept(writer);

    m_send(reinterpret_cast<const uint8_t *>(buffer.GetString()),
           buffer.GetSize(), false);
    return;
  }

  auto error_msg = "unknown function: " + function_name;
  m_send(reinterpret_cast<const uint8_t *>(error_msg.c_str()), error_msg.size(),
         false);
}

auto AnnotationService::apply_level_set(
    const std::vector<voxer::Annotation> &annotations,
    const std::string &dataset_id, StructuredGrid::Axis axis,
    uint32_t index) const -> std::vector<voxer::Annotation> {
  assert(m_datasets != nullptr);

  auto dataset = m_datasets->get(dataset_id);
  if (!dataset) {
    throw std::runtime_error("cannot find dataset " + dataset_id);
  }

  auto slice = dataset->get_slice(axis, index);

  std::vector<Annotation> result{};
  result.reserve(annotations.size());

  AnnotationLevelSetFilter filter{};
  for (auto &input : annotations) {
    result.emplace_back(filter.process(input, slice));
  }

  return result;
}

} // namespace voxer::remote
