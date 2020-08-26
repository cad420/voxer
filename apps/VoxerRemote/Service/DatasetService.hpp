#pragma once
#include "Service/AbstractService.hpp"
#include "Store/DatasetStore.hpp"
#include <string>

namespace voxer::remote {

class DatasetService final : public AbstractService {
public:
  void on_message(const char *message, uint32_t size) final;

  [[nodiscard]] auto get_path() const -> std::string final {
    return "/datasets";
  }

  voxer::remote::DatasetStore *m_datasets = nullptr;

private:
  void load_dataset(const char *json, uint32_t size);
};

} // namespace voxer::remote
