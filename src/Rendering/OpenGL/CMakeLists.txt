find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL)
include(${PROJECT_SOURCE_DIR}/third_party/glm.cmake)

embed_shaders(
    CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/shaders.hpp.in
    TARGET ${CMAKE_CURRENT_SOURCE_DIR}/shaders.hpp
    DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders/
)

list(
    APPEND
    VOXER_BACKEND_GL_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGLVolumeRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VolumeCache.cpp
    ${PROJECT_SOURCE_DIR}/third_party/glad/src/glad.c
)

add_library(voxer_backend_opengl SHARED ${VOXER_BACKEND_GL_SRCS})
target_link_libraries(
    voxer_backend_opengl
    PUBLIC
    voxer_core
    OpenGL::OpenGL
    OpenGL::EGL
    ${CMAKE_DL_LIBS}
    PRIVATE
    glm::glm
    -static-libstdc++
)
target_include_directories(
    voxer_backend_opengl
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/third_party/glad/include
)
target_compile_options(voxer_backend_opengl PRIVATE -Wall -Wextra)
target_compile_features(voxer_backend_opengl PRIVATE cxx_std_14)

if (VOXER_INSTALL)
  install(
      TARGETS voxer_backend_opengl
      DESTINATION lib
      EXPORT voxer-targets
  )
endif ()