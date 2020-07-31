#pragma once
#include "service/Service.hpp"
#include <rapidjson/document.h>
#include <voxer/DatasetStore.hpp>

class SliceService final : public Service {
public:
  void on_message(const char *message, uint32_t size) final;

  [[nodiscard]] auto get_path() const -> std::string final { return "/slice"; }

  voxer::DatasetStore *m_datasets = nullptr;
};