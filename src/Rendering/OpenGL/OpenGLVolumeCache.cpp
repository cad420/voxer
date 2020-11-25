#include "Rendering/OpenGL/OpenGLVolumeCache.hpp"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <fmt/core.h>
#include <iostream>
#include <stdexcept>
#include <voxer/Filters/GradientFilter.hpp>

namespace {

const EGLint egl_config_attribs[] = {EGL_SURFACE_TYPE,
                                     EGL_PBUFFER_BIT,
                                     EGL_BLUE_SIZE,
                                     8,
                                     EGL_GREEN_SIZE,
                                     8,
                                     EGL_RED_SIZE,
                                     8,
                                     EGL_DEPTH_SIZE,
                                     8,
                                     EGL_RENDERABLE_TYPE,
                                     EGL_OPENGL_BIT,
                                     EGL_NONE};

const EGLint pbuffer_attribs[] = {
    EGL_WIDTH, 1024, EGL_HEIGHT, 1024, EGL_NONE,
};

GLuint create_dataset_texture(voxer::StructuredGrid &dataset) {
  auto &info = dataset.info;
  auto &dimension = info.dimensions;

  //  voxer::GradientFilter filter{};
  //  auto filtered = filter.process(dataset);

  GLuint volume_texture = 0;
  glGenTextures(1, &volume_texture);
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_3D, volume_texture);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, dimension[0], dimension[1],
               dimension[2], 0, GL_RED, GL_UNSIGNED_BYTE,
               dataset.buffer.data());

  return volume_texture;
}

} // namespace

namespace voxer {

OpenGLVolumeCache::OpenGLVolumeCache() noexcept {
  static const int MAX_DEVICES = 4;
  EGLDeviceEXT egl_devices[MAX_DEVICES];
  EGLint num_devices;

  auto eglQueryDevicesEXT =
      (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT");
  eglQueryDevicesEXT(4, egl_devices, &num_devices);

  auto eglGetPlatformDisplayEXT =
      (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress(
          "eglGetPlatformDisplayEXT");

  m_egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT,
                                           egl_devices[0], nullptr);

  EGLint major, minor;
  eglInitialize(m_egl_display, &major, &minor);

  EGLint num_configs;
  EGLConfig egl_config;

  eglChooseConfig(m_egl_display, egl_config_attribs, &egl_config, 1,
                  &num_configs);

  EGLSurface egl_surface =
      eglCreatePbufferSurface(m_egl_display, egl_config, pbuffer_attribs);

  eglBindAPI(EGL_OPENGL_API);

  const EGLint context_attri[] = {EGL_CONTEXT_MAJOR_VERSION,
                                  4,
                                  EGL_CONTEXT_MINOR_VERSION,
                                  6,
                                  EGL_CONTEXT_OPENGL_PROFILE_MASK,
                                  EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
                                  EGL_NONE};
  m_egl_context = eglCreateContext(m_egl_display, egl_config, EGL_NO_CONTEXT,
                                   context_attri);

  eglMakeCurrent(m_egl_display, egl_surface, egl_surface, m_egl_context);

  if (!gladLoadGLLoader((void *(*)(const char *))(&eglGetProcAddress))) {
    std::cerr << "Failed to load OpenGL\n";
    std::terminate();
  }

  fmt::print(stdout,
             "OpenGLVolumeCache: Detected {} devices, using first one, OpenGL "
             "version {}.{}",
             num_devices, GLVersion.major, GLVersion.minor);
}

OpenGLVolumeCache::~OpenGLVolumeCache() noexcept {
  eglTerminate(m_egl_display);
}

auto OpenGLVolumeCache::get_instance() -> OpenGLVolumeCache * {
  static OpenGLVolumeCache instance{};

  return &instance;
}

auto OpenGLVolumeCache::get_context() -> EGLContext { return m_egl_context; }

auto OpenGLVolumeCache::get(StructuredGrid *data) -> GLuint {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_textures.has(data)) {
    auto item = m_textures.get(data);
    return item->value();
  }

  auto texture = create_dataset_texture(*data);

  m_textures.emplace(data,
                     OpenGLManagedResource(texture, data->shared_from_this()));

  return texture;
}

void OpenGLVolumeCache::load(StructuredGrid *data) {
  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_textures.has(data)) {
    return;
  }

  auto texture = create_dataset_texture(*data);

  m_textures.emplace(data,
                     OpenGLManagedResource(texture, data->shared_from_this()));
}

bool OpenGLVolumeCache::has(StructuredGrid *data) noexcept {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_textures.has(data);
}

} // namespace voxer