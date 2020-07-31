#pragma once
#include "service/Service.hpp"
#include "utils.hpp"
#include <future>
#include <memory>
#include <voxer/VolumeRenderer.hpp>

class VolumeRenderingService final : public Service {
public:
  struct Command {
    voxer::VolumeRenderer::Type engine = voxer::VolumeRenderer::Type::OSPRay;
    voxer::Scene scene{};
  };

  VolumeRenderingService();

  void on_message(const char *message, uint32_t size) final;

  [[nodiscard]] auto get_path() const -> std::string final {
    return "/rendering";
  }

  voxer::DatasetStore *m_datasets = nullptr;

private:
  std::unique_ptr<voxer::VolumeRenderer> m_opengl;
  std::unique_ptr<voxer::VolumeRenderer> m_ospray;

  auto parse(const char *message, uint32_t size) -> Command;
};