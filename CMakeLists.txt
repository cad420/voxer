cmake_minimum_required(VERSION 3.1)
project(VovisRenderer CXX)

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

find_package(ospray REQUIRED)
find_package(Poco REQUIRED COMPONENTS Net Util)
find_package(MPI REQUIRED)
find_package(VTK REQUIRED COMPONENTS vtkRenderingVolumeOpenGL2 vtkIOImage)
include(${VTK_USE_FILE})

# find tbb
find_library(
  TBB_LIB
  NAMES tbb
  HINTS ${TBB_LIB_PATH} ${OSPRAY_LIB_PATH} "${CMAKE_PREFIX_PATH}/lib"
)
if (NOT TBB_LIB)
  unset(TBB_LIB CACHE)
  message(FATAL_ERROR "tbb not found, please set TBB_LIB_PATH.")
else()
  unset(TBB_LIB_PATH CACHE)
  list(APPEND TBB_LIBRARIES ${TBB_LIB})
endif()
find_library(
  TBB_MALLOC_LIB
  NAMES tbbmalloc
  HINTS ${TBB_LIB_PATH} ${OSPRAY_LIB_PATH} "${CMAKE_PREFIX_PATH}/lib"
)
list(APPEND TBB_LIBRARIES ${TBB_MALLOC_LIB})

add_subdirectory(ParallelRenderer)
add_subdirectory(app)
add_subdirectory(test)
enable_testing()
add_test(NAME TestVolumeRendering COMMAND Test ${CMAKE_CURRENT_SOURCE_DIR}/test/configs/volume.json)