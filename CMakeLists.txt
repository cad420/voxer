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

add_subdirectory(src)

option(VOXER_BUILD_SERVER "whether to build voxer server" ON)
if (VOXER_BUILD_SERVER)
  add_subdirectory(apps/render_server)
endif ()

option(VOXER_BUILD_TESTS "whether to build tests" OFF)
if (VOXER_BUILD_TESTS)
  add_subdirectory(tests)
endif ()
