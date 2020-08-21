cmake_minimum_required(VERSION 3.11)

project(VoxerRemote LANGUAGES CXX C VERSION 0.1.0)

include(${PROJECT_SOURCE_DIR}/third_party/uWebSockets.cmake)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(
    VoxerRemote
    main.cpp
    Service/DatasetService.cpp
    Service/SliceService.cpp
    Service/VolumeRenderingService.cpp
    Store/DatasetStore.cpp
    Store/PipelineStore.cpp
)
target_include_directories(
    VoxerRemote
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    VoxerRemote
    PRIVATE
    voxer::voxer_core
    Threads::Threads
    ZLIB::ZLIB
    uWebSockets
    fmt::fmt
)
target_compile_features(VoxerRemote PRIVATE cxx_std_17)

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
install(
    TARGETS VoxerRemote
    DESTINATION bin
)
