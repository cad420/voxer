#include "Renderers/IRenderer.hpp"
#include <chrono>
#include <dlfcn.h>
#include <functional>
#include <memory>
#include <spdlog/spdlog.h>
#include <voxer/Renderers/VolumeRenderer.hpp>

using namespace std;

namespace voxer {

std::unordered_map<std::string, GetRenderingBackend> VolumeRenderer::symbols{};

VolumeRenderer::~VolumeRenderer() = default;

VolumeRenderer::VolumeRenderer(const char *backend) {
  m_backend = backend;
  auto it = symbols.find(backend);

  string postfix;
#ifndef NDEBUG
  postfix = "d";
#endif

  std::function<VoxerIRenderer *()> get_backend = nullptr;
  if (it != symbols.end()) {
    get_backend = it->second;
  } else {
    auto lib_name = string("libvoxer_backend_") + backend + postfix + ".so";
    void *lib = dlopen(lib_name.c_str(), RTLD_NOW);
    if (lib == nullptr) {
      throw runtime_error(dlerror());
    }

    void *symbol = dlsym(lib, "voxer_get_backend");
    if (symbol == nullptr) {
      throw runtime_error("Cannot find symbol `voxer_get_backend` in " +
                          lib_name);
    }

    get_backend = reinterpret_cast<GetRenderingBackend>(symbol);
    symbols.emplace(string(backend),
                    reinterpret_cast<GetRenderingBackend>(symbol));
    spdlog::info("rendering backend loaded: {}", lib_name);
  }

  auto context = get_backend();
  m_impl.reset(context);
}

void VolumeRenderer::render() const {
  if (m_impl == nullptr) {
    return;
  }

  auto start = chrono::steady_clock::now();

  m_impl->render();

  const auto delta = chrono::duration_cast<chrono::milliseconds>(
      chrono::steady_clock::now() - start);
  spdlog::debug(to_string(delta.count()) + " ms");
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

auto VolumeRenderer::get_backend() const noexcept -> const char * {
  return m_backend.c_str();
}

bool VolumeRenderer::has_cache(StructuredGrid *data) const noexcept {
  return m_impl->has_cache(data);
}

} // namespace voxer