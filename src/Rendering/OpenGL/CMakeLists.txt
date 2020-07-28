find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL)

embed_shaders(
    CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/shaders.hpp.in
    TARGET ${CMAKE_CURRENT_SOURCE_DIR}/shaders.hpp
    DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders/
)

list(
    APPEND
    VOXER_BACKEND_GL_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderingContextOpenGL.cpp
    ${PROJECT_SOURCE_DIR}/third_party/glad/src/glad.c
)

add_library(voxer_backend_gl SHARED ${VOXER_BACKEND_GL_SRCS})
target_link_libraries(
    voxer_backend_gl
    PUBLIC
    voxer_core
    OpenGL::OpenGL
    OpenGL::EGL
    ${CMAKE_DL_LIBS}
    PRIVATE
    vmcore
    -static-libstdc++
)
target_include_directories(
    voxer_backend_gl
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/third_party/glad/include
)
target_compile_options(voxer_backend_gl PRIVATE -Wall -Wextra)
target_compile_features(voxer_backend_gl PRIVATE cxx_std_14)

install(
    TARGETS voxer_backend_gl
    DESTINATION lib
    EXPORT voxer-targets
)