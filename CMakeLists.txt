cmake_minimum_required(VERSION 3.12)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

project(voxer
  LANGUAGES CXX
  VERSION 0.0.1
)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

add_subdirectory(src)

set(VOXER_BUILD_SERVER TRUE CACHE BOOL "whether to build voxer server")
if (VOXER_BUILD_SERVER)
  add_subdirectory(apps/server)
endif()

set(VOXER_BUILD_TESTS FALSE CACHE BOOL "whether to build tests")
if(VOXER_BUILD_TESTS)
  add_subdirectory(tests)
  enable_testing()
  # add_test(NAME TestVolumeRendering COMMAND Test ${CMAKE_CURRENT_SOURCE_DIR}/test/configs/volume.json)
endif()
