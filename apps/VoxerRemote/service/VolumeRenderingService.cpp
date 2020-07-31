#include "service/VolumeRenderingService.hpp"
#include <fmt/format.h>
#include <seria/deserialize.hpp>
#include <voxer/VolumeRenderer.hpp>
#include <voxer/utils.hpp>

using namespace std;
using namespace voxer;
using namespace fmt;

VolumeRenderingService::VolumeRenderingService() {
  m_opengl = make_unique<VolumeRenderer>(VolumeRenderer::Type::OpenGL);
  m_ospray = make_unique<VolumeRenderer>(VolumeRenderer::Type::OSPRay);
}

void VolumeRenderingService::on_message(const char *message, uint32_t size) {
  if (m_datasets == nullptr || m_send == nullptr)
    return;

  try {
    auto command = parse(message, size);
    auto &renderer =
        command.engine == VolumeRenderer::Type::OSPRay ? *m_ospray : *m_opengl;
    renderer.render(command.scene, *m_datasets);
    auto &image = renderer.get_colors();
    auto compressed = Image::encode(image, Image::Format::JPEG);
    m_send(reinterpret_cast<const uint8_t *>(compressed.data.data()),
           compressed.data.size(), true);
  } catch (exception &e) {
    auto error_msg = fmt::format(R"({{"error": "{}"}})", e.what());
    m_send(reinterpret_cast<const uint8_t *>(error_msg.data()),
           error_msg.size(), false);
  }
}

auto VolumeRenderingService::parse(const char *message, uint32_t size)
    -> Command {
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

  auto engine = VolumeRenderer::Type::OSPRay;
  it = json.FindMember("engine");
  if (it != json.end() && it->value.IsString()) {
    auto type = it->value.GetString();
    if (strcmp(type, "OpenGL") == 0) {
      engine = VolumeRenderer::Type::OpenGL;
    } else if (strcmp(type, "OSPRay") == 0) {
      engine = VolumeRenderer::Type::OSPRay;
    } else {
      throw runtime_error("Unsupported rendering engine: " + string(type));
    }
  }

  it = json.FindMember("params");
  if (it == json.end()) {
    throw JSON_error("params", "required");
  }
  auto params = it->value.GetObject();

  if (command_type == "render") {
    return {engine, Scene::deserialize(params)};
  }

  throw runtime_error("invalid JSON: unknown command type");
}