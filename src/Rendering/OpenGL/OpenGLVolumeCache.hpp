#pragma once
#define MESA_EGL_NO_X11_HEADERS
#define EGL_NO_X11
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <glad/glad.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

class OpenGLVolumeCache {
public:
  auto get(StructuredGrid *data) -> GLuint;
  void load(StructuredGrid *data);
  bool has(StructuredGrid *data);
  auto get_context() -> EGLContext;

  static auto get_instance() -> OpenGLVolumeCache *;

private:
  OpenGLVolumeCache() noexcept;
  ~OpenGLVolumeCache() noexcept;

  std::mutex m_mutex;
  std::unordered_map<StructuredGrid *, GLuint> m_textures;
  EGLDisplay m_egl_display{};
  EGLContext m_egl_context{};
};

} // namespace voxer
