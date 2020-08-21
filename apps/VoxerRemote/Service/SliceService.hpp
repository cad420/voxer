#pragma once
#include "Service/AbstractService.hpp"
#include "Store/DatasetStore.hpp"
#include <rapidjson/document.h>
#include <voxer/Data/Annotation.hpp>
#include <voxer/Data/Slice.hpp>

class SliceService final : public AbstractService {
public:
  void on_message(const char *message, uint32_t size) final;

  [[nodiscard]] auto get_path() const -> std::string final { return "/slice"; }

  voxer::DatasetStore *m_datasets = nullptr;
  std::unordered_map<voxer::SliceInfo, voxer::AnnotatedSliceInfo>
      annotation_store;
};