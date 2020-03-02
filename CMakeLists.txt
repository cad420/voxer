cmake_minimum_required(VERSION 3.10)

if (DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif ()

project(
    voxer
    LANGUAGES CXX
    VERSION 0.1.0
)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
SET(CMAKE_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib")

if (UNIX)
  #  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined -g")
  #  set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined -g")
endif ()
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

add_subdirectory(src)

option(VOXER_BUILD_SERVER "whether to build voxer server" ON)
if (VOXER_BUILD_SERVER)
  execute_process(COMMAND make WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party/uSockets)
  execute_process(COMMAND cp uSockets.a libuSockets.a WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party/uSockets)
  execute_process(COMMAND cp -r src uwebsockets WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party/uWebSockets)
  add_subdirectory(third_party/fmt)
  add_subdirectory(apps/server)
endif ()

option(VOXER_BUILD_TESTS "whether to build tests" OFF)
if (VOXER_BUILD_TESTS)
  add_subdirectory(tests)
endif ()
