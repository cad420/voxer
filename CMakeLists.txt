cmake_minimum_required(VERSION 3.11)

project(
    voxer
    LANGUAGES CXX C
    VERSION 0.1.0
)

option(VOXER_INSTALL "create target to install voxer library and headers" ON)
option(VOXER_BUILD_BACKEND_OPENGL "whether to build OpenGL rendering backend " ON)
option(VOXER_BUILD_BACKEND_OSPRAY "whether to build OSPRay rendering backend" ON)
option(VOXER_BUILD_SERVER "whether to build voxer server" ON)
option(VOXER_BUILD_PYBIND "whether to build python binding" OFF)
option(VOXER_BUILD_TESTS "whether to build tests" OFF)

# global settings
include(cmake/utils.cmake)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(FETCHCONTENT_QUIET FALSE)
SET(CMAKE_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib")

add_subdirectory(src)

if (VOXER_BUILD_SERVER)
  add_subdirectory(apps/VoxerRemote)
endif ()

if (VOXER_BUILD_PYBIND)
  add_subdirectory(apps/pyvoxer)
endif ()

if (VOXER_BUILD_TESTS)
  add_subdirectory(tests)
endif ()
