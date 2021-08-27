include(${PROJECT_SOURCE_DIR}/third_party/sdl2.cmake)

list(
        APPEND
        VoxerRenderer_SRCS
        src/main.cpp
        src/VoxerRenderer.cpp
)
add_executable(VoxerRenderer ${VoxerRenderer_SRCS})
target_link_libraries(VoxerRenderer
        PRIVATE
        voxer_core
        SDL2-static
        spdlog::spdlog
        glm::glm
        )