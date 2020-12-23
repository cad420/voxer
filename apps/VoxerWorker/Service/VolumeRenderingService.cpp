#include "Service/VolumeRenderingService.hpp"
#include <fmt/format.h>
#include <seria/deserialize.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <voxer/Rendering/VolumeRenderer.hpp>

using namespace std;
using namespace voxer;

namespace voxer::remote {

VolumeRenderingService::VolumeRenderingService() = default;

void VolumeRenderingService::on_message(
    const char *message, uint32_t size,
    const MessageCallback &callback) noexcept {
  assert(m_datasets != nullptr && callback != nullptr);

  if (m_datasets == nullptr) {
    return;
  }

  const char *env = getenv("LOG_LEVEL");
  auto log_time = env != nullptr && strlen(env) > 0;

  try {
    spdlog::stopwatch sw{};

    m_document.Parse(message, size);
    if (!m_document.IsObject()) {
      throw JSON_error("root", "object");
    }
    auto json = m_document.GetObject();
    auto it = json.FindMember("method");
    if (it == json.end() || !(it->value.IsString())) {
      throw JSON_error("method", "string");
    }

    string method = it->value.GetString();
    if (method == "render") {
      it = json.FindMember("params");
      if (it == json.end()) {
        throw JSON_error("params", "required");
      }
      auto params = it->value.GetObject();

      Scene scene{};
      seria::deserialize(scene, params);
      render(scene, callback);
    } else {
      throw runtime_error(fmt::format("unknown method: {}", method));
    }

    if (log_time) {
      spdlog::info("{} elapsed {}", method, sw);
    }
  } catch (exception &e) {
    auto error_msg = fmt::format(R"({{"error": "{}"}})", e.what());
    callback(reinterpret_cast<const uint8_t *>(error_msg.data()),
             error_msg.size(), false);
  }
}

void VolumeRenderingService::render(const Scene &scene,
                                    const MessageCallback &callback) {
  assert(m_datasets != nullptr && callback != nullptr);

  auto renderer = scene.renderer;
  std::transform(renderer.begin(), renderer.end(), renderer.begin(),
                 [](unsigned char c){ return std::tolower(c); });
  if (m_renderer == nullptr || m_renderer->get_backend() != renderer) {
    m_renderer = make_unique<VolumeRenderer>(renderer.c_str());
  }

  m_renderer->set_camera(scene.camera);
  m_renderer->set_background(scene.background[0], scene.background[1],
                             scene.background[2]);

  unordered_map<uint32_t, shared_ptr<voxer::TransferFunction>> tfcns_map;

  for (auto &volume_desc : scene.volumes) {
    if (!volume_desc.render) {
      continue;
    }

    auto dataset = m_datasets->get(volume_desc.dataset);

    if (!m_renderer->has_cache(dataset.get())) {
      std::string error = R"({"error": "need to load"})";
      callback(reinterpret_cast<const uint8_t *>(error.c_str()), error.size(),
               false);
    }

    auto volume = make_shared<voxer::Volume>();
    volume->dataset = std::move(dataset);
    volume->spacing = volume_desc.spacing;

    if (tfcns_map.find(volume_desc.tfcn_idx) == tfcns_map.end()) {
      auto tfcn = make_shared<voxer::TransferFunction>();
      auto &tfcn_desc = scene.tfcns[volume_desc.tfcn_idx];
      for (auto &item : tfcn_desc) {
        voxer::RGBColor color;
        color.from_hex(item.color.data());
        tfcn->add_point(item.x, item.y, color.data);
      }
      auto res = tfcns_map.emplace(volume_desc.tfcn_idx, move(tfcn));
      volume->tfcn = res.first->second;
    } else {
      volume->tfcn = tfcns_map[volume_desc.tfcn_idx];
    }

    m_renderer->add_volume(volume);
  }

  for (auto &isosurface_desc : scene.isosurfaces) {
    if (!isosurface_desc.render) {
      continue;
    }

    auto dataset = m_datasets->get(isosurface_desc.dataset);

    if (!m_renderer->has_cache(dataset.get())) {
      std::string error = R"({"error": "need to load"})";
      callback(reinterpret_cast<const uint8_t *>(error.c_str()), error.size(),
               false);
    }

    auto isosurface = make_shared<voxer::Isosurface>();
    isosurface->dataset = std::move(dataset);
    isosurface->color.from_hex(isosurface_desc.color.c_str());
    isosurface->value = isosurface_desc.value;
    m_renderer->add_isosurface(isosurface);
  }

  m_renderer->render();
  auto &image = m_renderer->get_colors();
  auto compressed =
      Image::encode(image, Image::Format::JPEG, Image::Quality::MEDIUM, true);

  callback(reinterpret_cast<const uint8_t *>(compressed.data.data()),
           compressed.data.size(), true);

  m_renderer->clear_scene();
}

} // namespace voxer::remote