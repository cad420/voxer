#pragma once
#include "AbstractService.hpp"
#include "Store/DatasetStore.hpp"
#include <voxer/Data/Annotation.hpp>

namespace voxer::remote {

struct AnnotationLevelSetParams {
  std::string dataset;
  std::string axis;
  uint32_t index;
  std::vector<voxer::Annotation> annotations;
};

class AnnotationService : public AbstractService {
public:
  void on_message(const char *message, uint32_t size,
                  const MessageCallback &callback) noexcept override;

  [[nodiscard]] auto get_path() const noexcept -> std::string override {
    return "/annotations";
  };

  [[nodiscard]] auto get_protocol() const noexcept -> Protocol override {
    return Protocol::RPC;
  };

  DatasetStore *m_datasets = nullptr;

private:
  [[nodiscard]] auto
  apply_level_set(const std::vector<voxer::Annotation> &annotations,
                  const std::string &dataset_id, StructuredGrid::Axis axis,
                  uint32_t index) const -> std::vector<voxer::Annotation>;
};

} // namespace voxer::remote
