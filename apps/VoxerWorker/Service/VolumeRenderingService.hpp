#pragma once
#include "DataModel/Scene.hpp"
#include "Service/AbstractService.hpp"
#include "Store/DatasetStore.hpp"
#include <future>
#include <memory>
#include <voxer/Rendering/VolumeRenderer.hpp>

namespace voxer::remote {

class VolumeRenderingService : public AbstractService {
public:
  VolumeRenderingService();

  void on_message(const char *message, uint32_t size,
                  const MessageCallback &callback) noexcept override;

  [[nodiscard]] auto get_protocol() const noexcept -> Protocol override {
    return Protocol::WebSocket;
  };

  voxer::remote::DatasetStore *m_datasets = nullptr;
  void render(const Scene &scene,  const MessageCallback &callback);

private:
  std::unique_ptr<voxer::VolumeRenderer> m_renderer;


  Scene parse_scene(uint8_t *message, uint32_t size, std::string &id);
};

} // namespace voxer::remote
