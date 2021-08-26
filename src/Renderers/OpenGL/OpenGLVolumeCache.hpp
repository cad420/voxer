#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <voxer/Data/StructuredGrid.hpp>
#include <voxer/Util/LRUCache.hpp>
#ifndef _WINDOWS
#define MESA_EGL_NO_X11_HEADERS
#define EGL_NO_X11
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <glad/glad.h>
#else
//#include <gl/gl.h>
#include <glad/wgl.h>
#include <windows.h>

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc,
                                                     HGLRC hShareContext,
                                                     const int *attribList);
extern wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;

// See https://www.opengl.org/registry/specs/ARB/wgl_create_context.txt for all
// values
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

typedef BOOL WINAPI wglChoosePixelFormatARB_type(
    HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList,
    UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
extern wglChoosePixelFormatARB_type *wglChoosePixelFormatARB;

// See https://www.opengl.org/registry/specs/ARB/wgl_pixel_format.txt for all
// values
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023

#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_TYPE_RGBA_ARB 0x202B

inline static void fatal_error(const char *msg) {
  MessageBoxA(NULL, msg, "Error", MB_OK | MB_ICONEXCLAMATION);
  exit(EXIT_FAILURE);
}

inline static void init_opengl_extensions(void) {
  // Before we can load extensions, we need a dummy OpenGL context, created
  // using a dummy window. We use a dummy window because you can only set the
  // pixel format for a window once. For the real window, we want to use
  // wglChoosePixelFormatARB (so we can potentially specify options that aren't
  // available in PIXELFORMATDESCRIPTOR), but we can't load and use that before
  // we have a context.
  WNDCLASSA window_class = {
      .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
      .lpfnWndProc = DefWindowProcA,
      .hInstance = GetModuleHandle(0),
      .lpszClassName = "Dummy_WGL_djuasiodwa",
  };

  if (!RegisterClassA(&window_class)) {
    fatal_error("Failed to register dummy OpenGL window.");
  }

  HWND dummy_window =
      CreateWindowExA(0, window_class.lpszClassName, "Dummy OpenGL Window", 0,
                      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                      CW_USEDEFAULT, 0, 0, window_class.hInstance, 0);

  if (!dummy_window) {
    fatal_error("Failed to create dummy OpenGL window.");
  }

  HDC dummy_dc = GetDC(dummy_window);

  PIXELFORMATDESCRIPTOR pfd = {
      .nSize = sizeof(pfd),
      .nVersion = 1,
      .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
      .iPixelType = PFD_TYPE_RGBA,

      .cColorBits = 32,
      .cAlphaBits = 8,

      .cDepthBits = 24,
      .cStencilBits = 8,
      .iLayerType = PFD_MAIN_PLANE,
  };

  int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
  if (!pixel_format) {
    fatal_error("Failed to find a suitable pixel format.");
  }
  if (!SetPixelFormat(dummy_dc, pixel_format, &pfd)) {
    fatal_error("Failed to set the pixel format.");
  }

  HGLRC dummy_context = wglCreateContext(dummy_dc);

  if (!dummy_context) {
    fatal_error("Failed to create a dummy OpenGL rendering context.");
  }

  if (!wglMakeCurrent(dummy_dc, dummy_context)) {
    fatal_error("Failed to activate dummy OpenGL rendering context.");
  }

  wglCreateContextAttribsARB =
      (wglCreateContextAttribsARB_type *)wglGetProcAddress(
          "wglCreateContextAttribsARB");
  wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type *)wglGetProcAddress(
      "wglChoosePixelFormatARB");

  wglMakeCurrent(dummy_dc, 0);
  wglDeleteContext(dummy_context);
  ReleaseDC(dummy_window, dummy_dc);
  DestroyWindow(dummy_window);

  UnregisterClassA(window_class.lpszClassName, window_class.hInstance);
}

inline static HGLRC init_opengl(HDC real_dc, HGLRC ctx = NULL) {
  init_opengl_extensions();

  // Now we can choose a pixel format the modern way, using
  // wglChoosePixelFormatARB.
  int pixel_format_attribs[] = {WGL_DRAW_TO_WINDOW_ARB,
                                GL_TRUE,
                                WGL_SUPPORT_OPENGL_ARB,
                                GL_TRUE,
                                WGL_DOUBLE_BUFFER_ARB,
                                GL_TRUE,
                                WGL_ACCELERATION_ARB,
                                WGL_FULL_ACCELERATION_ARB,
                                WGL_PIXEL_TYPE_ARB,
                                WGL_TYPE_RGBA_ARB,
                                WGL_COLOR_BITS_ARB,
                                32,
                                WGL_DEPTH_BITS_ARB,
                                24,
                                WGL_STENCIL_BITS_ARB,
                                8,
                                0};

  int pixel_format;
  UINT num_formats;
  wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, &pixel_format,
                          &num_formats);
  if (!num_formats) {
    fatal_error("Failed to set the OpenGL 3.3 pixel format.");
  }

  PIXELFORMATDESCRIPTOR pfd;
  DescribePixelFormat(real_dc, pixel_format, sizeof(pfd), &pfd);
  if (!SetPixelFormat(real_dc, pixel_format, &pfd)) {
    fatal_error("Failed to set the OpenGL 3.3 pixel format.");
  }

  gladLoaderLoadWGL(real_dc);
  // Specify that we want to create an OpenGL 3.3 core profile context
  int gl33_attribs[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB,
      4,
      WGL_CONTEXT_MINOR_VERSION_ARB,
      6,
      WGL_CONTEXT_PROFILE_MASK_ARB,
      WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0,
  };

  HGLRC gl33_context = wglCreateContextAttribsARB(real_dc, ctx, gl33_attribs);
  if (!gl33_context) {
    fatal_error("Failed to create OpenGL 3.3 context.");
  }

  if (!wglMakeCurrent(real_dc, gl33_context)) {
    fatal_error("Failed to activate OpenGL 3.3 rendering context.");
  }

  if (!gladLoaderLoadGL()) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(gl33_context);
  }

  return gl33_context;
}

inline static LRESULT CALLBACK window_callback(HWND window, UINT msg,
                                               WPARAM wparam, LPARAM lparam) {
  LRESULT result = 0;

  switch (msg) {
  case WM_CLOSE:
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    result = DefWindowProcA(window, msg, wparam, lparam);
    break;
  }

  return result;
}

inline static HWND create_window(HINSTANCE inst, LPCSTR name) {
  WNDCLASSA window_class = {
      .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
      .lpfnWndProc = window_callback,
      .hInstance = inst,
      .hCursor = LoadCursor(0, IDC_ARROW),
      .hbrBackground = 0,
      .lpszClassName = name,
  };

  if (!RegisterClassA(&window_class)) {
    DWORD err = GetLastError();
    fatal_error("Failed to register window.");
  }

  // Specify a desired width and height, then adjust the rect so the window's
  // client area will be that size.
  RECT rect = {
      .right = 1920,
      .bottom = 1080,
  };
  DWORD window_style = WS_OVERLAPPEDWINDOW;
  AdjustWindowRect(&rect, window_style, false);

  HWND window =
      CreateWindowExA(0, window_class.lpszClassName, "OpenGL", window_style,
                      CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left,
                      rect.bottom - rect.top, 0, 0, inst, 0);

  if (!window) {
    fatal_error("Failed to create window.");
  }
  UnregisterClassA(name, inst);
  return window;
}

#endif

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

#ifndef _WINDOWS
  auto get_context() -> EGLContext;
  EGLDisplay get_display();
#else
  auto get_context() -> HGLRC { return m_wgl_context; }
  auto get_handle() -> HDC { return m_wgl_window_handle; }
#endif

  static auto get_instance() -> OpenGLVolumeCache *;

private:
  OpenGLVolumeCache() noexcept;
  ~OpenGLVolumeCache() noexcept;

  std::mutex m_mutex;
  LRUCache<StructuredGrid *, OpenGLManagedResource> m_textures;

#ifndef _WINDOWS
  EGLDisplay m_egl_display{};
  EGLContext m_egl_context{};
#else
  HDC m_wgl_window_handle;
  HGLRC m_wgl_context;
#endif
};

} // namespace voxer
