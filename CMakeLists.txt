cmake_minimum_required(VERSION 3.11)

project(
    voxer
    LANGUAGES CXX C
    VERSION 0.1.0
)

if(WIN32)
  add_compile_definitions(WINDOWS)
elseif(UNIX AND NOT APPLE)
  if(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
    add_compile_definitions(LINUX)
  endif()
endif()

option(VOXER_BUILD_RENDERER_OPENGL "build OpenGL renderer" ON)
option(VOXER_BUILD_RENDERER_OSPRAY "build OSPRay renderer" ON)
option(VOXER_BUILD_ANNOTATION_MODULE "build annotation related module" ON)
option(VOXER_BUILD_SERVER "whether to build voxer server" ON)
option(VOXER_BUILD_PYBIND "whether to build python binding" OFF)
option(VOXER_BUILD_VOLUME_TO_VIDEO "build volume to video application" OFF)
option(VOXER_BUILD_TESTS "whether to build tests" OFF)
option(VOXER_INSTALL "create target to install voxer library and headers" ON)

# global settings
include(cmake/utils.cmake)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(FETCHCONTENT_QUIET FALSE)
SET(CMAKE_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib")

add_subdirectory(src)

if (VOXER_BUILD_SERVER)
  add_subdirectory(apps/VoxerWorker)
endif ()

if (VOXER_BUILD_VOLUME_TO_VIDEO)
  add_subdirectory(apps/VolumeToVideo)
endif ()

if (VOXER_BUILD_PYBIND)
  add_subdirectory(apps/pyvoxer)
endif ()

if (VOXER_BUILD_TESTS)
  add_subdirectory(tests)
endif ()
