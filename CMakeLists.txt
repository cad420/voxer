cmake_minimum_required(VERSION 3.12)

project(VoxerSevice
  LANGUAGES CXX
  VERSION 0.0.1
)

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

add_subdirectory(voxer)
add_subdirectory(server)

if(VOXER_BUILD_TESTS)
  add_subdirectory(tests)
endif()

# enable_testing()
# add_test(NAME TestVolumeRendering COMMAND Test ${CMAKE_CURRENT_SOURCE_DIR}/test/configs/volume.json)