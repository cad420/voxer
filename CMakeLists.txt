cmake_minimum_required(VERSION 3.1)
project(VovisRenderer)

set(CMAKE_CXX_STANDARD 11)
set(CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")
IF(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Options: Debug Release RelWithDebInfo." FORCE)
ENDIF()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CONFIGURATION_TYPES})

set(OSPRAY_LIB_PATH "" CACHE PATH "Path of ospray library.")
set(POCO_LIB_PATH "" CACHE PATH "Path of poco library.")
set(MPI_LIB_PATH "" CACHE PATH "Path of mpi library.")
set(EMBREE_LIB_PATH "" CACHE PATH "Path of embree library.")
set(TBB_LIB_PATH "" CACHE PATH "Path of tbb library.")

# find ospray
find_library(
  OSPRAY_LIB
  NAMES ospray
  HINTS ${OSPRAY_LIB_PATH} "${CMAKE_PREFIX_PATH}/lib"
)
if (NOT OSPRAY_LIB)
  message(FATAL_ERROR "ospray not found, please set OSPRAY_LIB_PATH.")
else()
  unset(OSPRAY_LIB_PATH CACHE)
endif()

find_library(
  OSPRAY_LIB_COMMON
  NAMES ospray_common
  HINTS ${OSPRAY_LIB_PATH} "${CMAKE_PREFIX_PATH}/lib"
)
find_library(
  OSPRAY_LIB_MPI
  NAMES ospray_mpi_common
  HINTS ${OSPRAY_LIB_PATH} "${CMAKE_PREFIX_PATH}/lib"
)

set(OSPRAY_INCLUDE_DIR "" CACHE PATH "Path of ospray header files.")
find_path(
  OSPRAY_INCLUDE_PATH
  NAMES ospray.h ospray/ospray.h
  HINTS ${OSPRAY_INCLUDE_DIR} "${CMAKE_PREFIX_PATH}/include/"
)
if (OSPRAY_INCLUDE_PATH-NOTFOUND)
  message(FATAL_ERROR "OSPRay header files not found, please set OSPRAY_INCLUDE_DIR.")
else()
  unset(OSPRAY_INCLUDE_DIR CACHE)
endif()

# find embree
find_library(
  EMBREE_LIB
  NAMES embree3 embree
  HINTS "${CMAKE_PREFIX_PATH}/lib" ${EMBREE_LIB_PATH}
)
if (NOT EMBREE_LIB)
  unset(EMBREE_LIB CACHE)
  message(FATAL_ERROR "embree not found, please set EMBREE_LIB_PATH.")
else()
  unset(EMBREE_LIB_PATH CACHE)
endif()

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
endif()
find_library(
  TBB_MALLOC_LIB
  NAMES tbbmalloc
  HINTS ${TBB_LIB_PATH} ${OSPRAY_LIB_PATH} "${CMAKE_PREFIX_PATH}/lib"
)

# find mpi
find_package(MPI REQUIRED)
if (NOT MPI_CXX_FOUND)
  unset(MPI_LIB CACHE)
  message(FATAL_ERROR "mpi not found, please set MPI_CXX_COMPILER to the MPI wrapper (mpicc, etc.).")
endif()

# find poco
find_library(
  POCO_FOUNDATION_LIB
  NAMES PocoFoundation
  HINTS ${POCO_LIB_PATH} "${CMAKE_PREFIX_PATH}/lib"
)
find_library(
  POCO_NET_LIB
  NAMES PocoNet
  HINTS ${POCO_LIB_PATH} "${CMAKE_PREFIX_PATH}/lib"
)
find_library(
  POCO_UTIL_LIB
  NAMES PocoUtil
  HINTS ${POCO_LIB_PATH} "${CMAKE_PREFIX_PATH}/lib"
)
if (NOT POCO_NET_LIB)
  unset(POCO_NET_LIB CACHE)
  message(FATAL_ERROR "Poco not found, please set POCO_LIB_PATH.")
else()
  unset(POCO_LIB_PATH CACHE)
endif()

set(POCO_INCLUDE_PATH "" CACHE PATH "Path of poco header files.")
find_path(
  POCO_INCLUDE_PATH
  NAMES Poco.h Poco/Poco.h
  HINTS ${POCO_INCLUDE_DIR} "${CMAKE_PREFIX_PATH}/include"
)
if (POCO_INCLUDE_PATH-NOTFOUND)
  message(FATAL_ERROR "Poco header files not found, please set POCO_INCLUDE_DIR.")
else()
  unset(POCO_INCLUDE_DIR CACHE)
endif()

file(GLOB_RECURSE VOVIS_FILES ${PROJECT_SOURCE_DIR}/ParallelRenderer/*.cpp)
file(GLOB_RECURSE VOVIS_CONFIG_FILES ${PROJECT_SOURCE_DIR}/ParallelRenderer/config/*.cpp)
file(GLOB_RECURSE VOVIS_DATA_FILES ${PROJECT_SOURCE_DIR}/ParallelRenderer/data/*.cpp)
file(GLOB_RECURSE RAWREADER_FILES ${PROJECT_SOURCE_DIR}/third_party/RawReader/*.cpp)
add_executable(VovisRenderer ${VOVIS_FILES} ${RAWREADER_FILES})
target_include_directories(
  VovisRenderer
  PUBLIC
  ${OSPRAY_INCLUDE_PATH}
  ${OSPRAY_INCLUDE_PATH}/ospray
  ${MPI_CXX_INCLUDE_PATH}
  ${POCO_INCLUDE_PATH}
  ${PROJECT_SOURCE_DIR}
)
target_link_libraries(
  VovisRenderer
  ${OSPRAY_LIB}
  ${OSPRAY_LIB_COMMON}
  ${OSPRAY_LIB_MPI}
  ${EMBREE_LIB}
  ${TBB_LIB}
  ${TBB_MALLOC_LIB}
  ${MPI_CXX_LIBRARIES}
  ${POCO_FOUNDATION_LIB}
  ${POCO_NET_LIB}
  ${POCO_UTIL_LIB}
)