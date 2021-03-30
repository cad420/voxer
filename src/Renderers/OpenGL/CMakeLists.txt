find_package(OpenGL REQUIRED)
include(${PROJECT_SOURCE_DIR}/third_party/glm.cmake)

embed_shaders(
    CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/shaders.hpp.in
    TARGET ${CMAKE_CURRENT_SOURCE_DIR}/shaders.hpp
    DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders/
)

if (UNIX)
    list(
        APPEND
        VOXER_BACKEND_GL_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/OpenGLVolumeRenderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OpenGLVolumeCache.cpp
        ${PROJECT_SOURCE_DIR}/third_party/glad/src/glad.c
    )
else()
    list(
        APPEND
        VOXER_BACKEND_GL_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/OpenGLVolumeRenderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OpenGLVolumeCache.cpp
        ${PROJECT_SOURCE_DIR}/third_party/glad_wgl/src/wgl.c
        ${PROJECT_SOURCE_DIR}/third_party/glad_wgl/src/gl.c
    )
endif()

add_library(voxer_renderer_opengl SHARED ${VOXER_BACKEND_GL_SRCS})
target_link_libraries(
    voxer_renderer_opengl
    PUBLIC
    voxer_core
    ${OPENGL_LIBRARIES}
    ${CMAKE_DL_LIBS}
    PRIVATE
    glm::glm
)
target_include_directories(
    voxer_renderer_opengl
    PRIVATE
        ${OPENGL_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/third_party/glad/include
        ${PROJECT_SOURCE_DIR}/third_party/glad_wgl/include/
)
if (UNIX)
    target_compile_features(voxer_renderer_opengl PRIVATE cxx_std_14)
    target_compile_options(voxer_renderer_opengl PRIVATE -Wall -Wextra)
else()
    target_compile_features(voxer_renderer_opengl PRIVATE cxx_std_20)
endif()

if (VOXER_INSTALL)
  install(
      TARGETS voxer_renderer_opengl
      DESTINATION lib
      EXPORT voxer-targets
  )
endif ()