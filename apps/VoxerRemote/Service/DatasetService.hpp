#pragma once
#include "Service/AbstractService.hpp"
#include "Store/DatasetStore.hpp"
#include <string>

namespace voxer::remote {

struct LoadDataSetParams {
  std::string id;
  std::string name;
  std::string path;
};

struct LoadDatasetResponse {
  std::string id;
  std::array<uint32_t, 3> dimensions{};
  std::vector<uint32_t> histogram{};
  std::array<float, 2> range{};
};

class DatasetService final : public AbstractService {
public:
  void on_message(const char *message, uint32_t size) noexcept final;

  [[nodiscard]] auto get_path() const noexcept -> std::string final {
    return "/datasets";
  }

  [[nodiscard]] auto get_protocol() const noexcept -> Protocol override {
    return Protocol::HTTP;
  };

  voxer::remote::DatasetStore *m_datasets = nullptr;

private:
  [[nodiscard]] auto load_dataset(const LoadDataSetParams &params) const
      -> LoadDatasetResponse;
};

} // namespace voxer::remote
