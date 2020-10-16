#include "Service/VolumeRenderingService.hpp"
#include <fmt/format.h>
#include <seria/deserialize.hpp>
#include <voxer/Rendering/VolumeRenderer.hpp>

using namespace std;
using namespace voxer;
using namespace fmt;

namespace voxer::remote {

VolumeRenderingService::VolumeRenderingService() {
  m_opengl = make_unique<VolumeRenderer>("opengl");
  m_ospray = make_unique<VolumeRenderer>("ospray");
}

void VolumeRenderingService::on_message(const char *message, uint32_t size) {
  if (m_datasets == nullptr || m_send == nullptr)
    return;

  try {
    auto command = parse(message, size);
    auto &renderer = command.first == "ospray" ? *m_ospray : *m_opengl;

    traverse_scene(renderer, command.second);

    renderer.render();
    auto &image = renderer.get_colors();

    auto compressed = Image::encode(image, Image::Format::JPEG);
    m_send(reinterpret_cast<const uint8_t *>(compressed.data.data()),
           compressed.data.size(), true);
    renderer.clear_scene();
  } catch (exception &e) {
    auto error_msg = fmt::format(R"({{"error": "{}"}})", e.what());
    m_send(reinterpret_cast<const uint8_t *>(error_msg.data()),
           error_msg.size(), false);
  }
}

auto VolumeRenderingService::parse(const char *message, uint32_t size)
    -> pair<string, Scene> {
  m_document.Parse(message, size);

  if (!m_document.IsObject()) {
    throw JSON_error("root", "object");
  }

  auto json = m_document.GetObject();

  auto it = json.FindMember("type");
  if (it == json.end() || !(it->value.IsString())) {
    throw JSON_error("type", "string");
  }
  string command_type = it->value.GetString();

  string engine;
  it = json.FindMember("engine");
  if (it != json.end() && it->value.IsString()) {
    auto type = it->value.GetString();
    if (strcmp(type, "OpenGL") == 0) {
      engine = "opengl";
    } else if (strcmp(type, "OSPRay") == 0) {
      engine = "ospray";
    } else {
      throw runtime_error("Unsupported rendering engine: " + string(type));
    }
  }

  it = json.FindMember("params");
  if (it == json.end()) {
    throw JSON_error("params", "required");
  }
  auto params = it->value.GetObject();

  voxer::remote::Scene scene{};
  seria::deserialize(scene, params);
  return make_pair(engine, scene);
}

void VolumeRenderingService::traverse_scene(VolumeRenderer &renderer,
                                            const Scene &scene) const {
  renderer.set_camera(scene.camera);
  renderer.set_background(scene.background[0], scene.background[1],
                          scene.background[2]);

  unordered_map<uint32_t, shared_ptr<voxer::TransferFunction>> tfcns_map;

  for (auto &volume_desc : scene.volumes) {
    if (!volume_desc.render) {
      continue;
    }

    auto &dataset = m_datasets->get(volume_desc.dataset);

    auto volume = make_shared<voxer::Volume>();
    volume->dataset = dataset;
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

    renderer.add_volume(volume);
  }

  for (auto &isosurface_desc : scene.isosurfaces) {
    if (!isosurface_desc.render) {
      continue;
    }

    auto &dataset = m_datasets->get(isosurface_desc.dataset);

    auto isosurface = make_shared<voxer::Isosurface>();
    isosurface->dataset = dataset;
    isosurface->color.from_hex(isosurface_desc.color.c_str());
    isosurface->value = isosurface_desc.value;
    renderer.add_isosurface(isosurface);
  }
}

} // namespace voxer::remote