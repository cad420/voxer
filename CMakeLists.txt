cmake_minimum_required(VERSION 3.11)

if (DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif ()

project(
    voxer
    LANGUAGES CXX C
    VERSION 0.1.0
)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)
SET(CMAKE_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib")

include(cmake/utils.cmake)

option(VOXER_INSTALL "install voxer library and headers" ON)
option(VOXER_BUILD_BACKEND_GL "whether to build OpenGL rendering backend " ON)
option(VOXER_BUILD_BACKEND_OSPRAY "whether to build OSPRay rendering backend" ON)
option(VOXER_BUILD_SERVER "whether to build voxer server" ON)
option(VOXER_BUILD_PYBIND "whether to build python binding" OFF)
option(VOXER_BUILD_TESTS "whether to build tests" OFF)

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
