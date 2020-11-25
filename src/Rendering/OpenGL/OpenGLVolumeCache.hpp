#pragma once
#define MESA_EGL_NO_X11_HEADERS
#define EGL_NO_X11
#define EGL_EGLEXT_PROTOTYPES
#include "Common/LRUCache.hpp"
#include <EGL/egl.h>
#include <glad/glad.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

class OpenGLManagedResource {
public:
  OpenGLManagedResource() = default;

  OpenGLManagedResource(GLuint texture,
                        std::shared_ptr<voxer::StructuredGrid> data)
      : m_data(std::move(data)), m_texture(texture) {}

  OpenGLManagedResource &operator=(OpenGLManagedResource &&rhs) noexcept {
    if (this == &rhs) {
      return *this;
    }

    m_data = std::move(rhs.m_data);
    m_texture = rhs.m_texture;
    rhs.m_texture = 0;
    return *this;
  }

  OpenGLManagedResource(OpenGLManagedResource &&rhs) noexcept
      : m_data(std::move(rhs.m_data)), m_texture(rhs.m_texture) {
    rhs.m_texture = 0;
  }

  ~OpenGLManagedResource() {
    if (m_texture != 0) {
      glDeleteTextures(1, &m_texture);
    }
  }

  GLuint value() const noexcept { return m_texture; }

private:
  std::shared_ptr<voxer::StructuredGrid> m_data;
  GLuint m_texture = 0;
};

class OpenGLVolumeCache {
public:
  auto get(StructuredGrid *data) -> GLuint;
  void load(StructuredGrid *data);
  bool has(StructuredGrid *data) noexcept;
  auto get_context() -> EGLContext;

  static auto get_instance() -> OpenGLVolumeCache *;

private:
  OpenGLVolumeCache() noexcept;
  ~OpenGLVolumeCache() noexcept;

  std::mutex m_mutex;
  LRUCache<StructuredGrid *, OpenGLManagedResource> m_textures;

  EGLDisplay m_egl_display{};
  EGLContext m_egl_context{};
};

} // namespace voxer
