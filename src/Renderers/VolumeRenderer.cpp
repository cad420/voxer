#include "Renderers/IRenderer.hpp"
#include <dlfcn.h>
#include <functional>
#include <memory>
#include <spdlog/spdlog.h>
#include <voxer/Renderers/VolumeRenderer.hpp>

using namespace std;

namespace voxer {

std::unordered_map<std::string, GetRenderingBackend> VolumeRenderer::symbols{};

VolumeRenderer::~VolumeRenderer() = default;

VolumeRenderer::VolumeRenderer(const char *renderer) {
  m_name = renderer;
  auto it = symbols.find(renderer);

  string postfix;
#ifndef NDEBUG
  postfix = "d";
#endif

  std::function<VoxerIRenderer *()> get_renderer = nullptr;
  if (it != symbols.end()) {
    get_renderer = it->second;
  } else {
    auto lib_name = string("libvoxer_renderer_") + renderer + postfix + ".so";
    void *lib = dlopen(lib_name.c_str(), RTLD_NOW);
    if (lib == nullptr) {
      throw runtime_error(dlerror());
    }

    void *symbol = dlsym(lib, "voxer_get_renderer");
    if (symbol == nullptr) {
      throw runtime_error("Cannot find symbol `voxer_get_renderer` in " +
                          lib_name);
    }

    get_renderer = reinterpret_cast<GetRenderingBackend>(symbol);
    symbols.emplace(string(renderer),
                    reinterpret_cast<GetRenderingBackend>(symbol));
    spdlog::info("rendering renderer loaded: {}", lib_name);
  }

  auto context = get_renderer();
  m_impl.reset(context);
}

void VolumeRenderer::render() const {
  if (m_impl == nullptr) {
    return;
  }

  // TODO: use memory pool to store image
  m_impl->render();
}

auto VolumeRenderer::get_colors() -> const Image & {
  return m_impl->get_colors();
}

void VolumeRenderer::set_camera(const Camera &camera) noexcept {
  m_impl->set_camera(camera);
}

void VolumeRenderer::add_volume(const std::shared_ptr<Volume> &volume) {
  return m_impl->add_volume(volume);
}

void VolumeRenderer::add_isosurface(
    const std::shared_ptr<Isosurface> &isosurface) {
  return m_impl->add_isosurface(isosurface);
}

void VolumeRenderer::clear_scene() { m_impl->clear_scene(); }

void VolumeRenderer::set_background(float r, float g, float b) noexcept {
  m_impl->set_background(r, g, b);
}

auto VolumeRenderer::get_name() const noexcept -> const char * {
  return m_name.c_str();
}

bool VolumeRenderer::has_cache(StructuredGrid *data) const noexcept {
  return m_impl->has_cache(data);
}

} // namespace voxer