#pragma once
#include "Service/AbstractService.hpp"
#include <rapidjson/document.h>
#include <voxer/Annotation.hpp>
#include <voxer/DatasetStore.hpp>
#include <voxer/Slice.hpp>

class SliceService final : public AbstractService {
public:
  void on_message(const char *message, uint32_t size) final;

  [[nodiscard]] auto get_path() const -> std::string final { return "/slice"; }

  voxer::DatasetStore *m_datasets = nullptr;
  std::unordered_map<voxer::SliceInfo, voxer::AnnotatedSliceInfo>
      annotation_store;
};