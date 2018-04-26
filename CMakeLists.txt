cmake_minimum_required(VERSION 3.1)
project(VovisRenderer)

set(CMAKE_CXX_STANDARD 11)
set(CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")
IF(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "DEBUG" CACHE STRING "Options: Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CONFIGURATION_TYPES})

set(OSPRAY_LIB_PATH "/usr/local/lib" CACHE PATH "Path of ospray library.")
set(OSPRAY_INCLUDE_PATH "/usr/local/include/ospray/" CACHE PATH "Path of ospray header files.")
set(EMBREE_LIB_PATH "/home/ukabuer/ospray/embree-2.17.2.x86_64.linux/lib/" CACHE PATH "Path of embree library.")
set(TBB_LIB_PATH "/home/ukabuer/ospray/tbb2018_20171205oss/lib/intel64/gcc4.7/" CACHE PATH "Path of tbb library.")
set(MPI_LIB_PATH "/usr/lib" CACHE PATH "Path of mpi library.")
set(MPI_INCLUDE_PATH "/usr/lib/mpich/include" CACHE PATH "Path of mpi header files.")

find_library(
  OSPRAY_LIB
  NAMES ospray ospray_common ospray_mpi_common
  HINTS "${CMAKE_PREFIX_PATH}/lib" ${OSPRAY_LIB}
)

find_library(
  OSPRAY_LIB_COMMON
  NAMES ospray_common
  HINTS "${CMAKE_PREFIX_PATH}/lib" ${OSPRAY_LIB}
)

find_library(
  OSPRAY_LIB_MPI
  NAMES ospray_mpi_common
  HINTS "${CMAKE_PREFIX_PATH}/lib" ${OSPRAY_LIB}
)

find_library(
  MPI_LIB
  NAMES mpi
  HINTS "${CMAKE_PREFIX_PATH}/lib" ${MPI_LIB_PATH}
)

find_library(
  EMBREE_LIB
  NAMES embree
  HINTS "${CMAKE_PREFIX_PATH}/lib" ${EMBREE_LIB_PATH}
)

find_library(
  TBB_LIB
  NAMES tbb
  HINTS "${CMAKE_PREFIX_PATH}/lib" ${TBB_LIB_PATH}
)

find_library(
  Z_LIB
  NAMES z
  HINTS "${CMAKE_PREFIX_PATH}/lib" ${Z_LIB}
)

find_library(
  SSL_LIB
  NAMES ssl
  HINTS "${CMAKE_PREFIX_PATH}/lib" ${SSL_LIB}
)

find_library(
  CRYPTO_LIB
  NAMES crypto
  HINTS "${CMAKE_PREFIX_PATH}/lib" ${CRYPTO_LIB}
)

if (NOT OSPRAY_LIB)
  unset(OSPRAY_LIB CACHE)
  message(FATAL_ERROR "ospray not found, please set OSPRAY_LIB_PATH.")
else()
  unset(OSPRAY_LIB_PATH CACHE)
endif()

if (NOT MPI_LIB)
  unset(MPI_LIB CACHE)
  message(FATAL_ERROR "mpi not found, please set MPI_LIB_PATH.")
else()
  unset(MPI_LIB_PATH CACHE)
endif()

if (NOT EMBREE_LIB)
  unset(EMBREE_LIB CACHE)
  message(FATAL_ERROR "embree not found, please set EMBREE_LIB_PATH.")
else()
  unset(EMBREE_LIB_PATH CACHE)
endif()

if (NOT TBB_LIB)
  unset(TBB_LIB CACHE)
  message(FATAL_ERROR "tbb not found, please set TBB_LIB_PATH.")
else()
  unset(TBB_LIB_PATH CACHE)
endif()

file(GLOB_RECURSE rawReader_FILES ${PROJECT_SOURCE_DIR}/third_party/RawReader/*.cpp)
file(GLOB_RECURSE uWS_FILES ${PROJECT_SOURCE_DIR}/third_party/uWebSockets/*.cpp)
add_executable(VovisRenderer ParallelRenderer/ParallelRenderer.cpp ParallelRenderer/GenerateSciVis.cpp ${rawReader_FILES} ${uWS_FILES})
target_include_directories(VovisRenderer PUBLIC ${OSPRAY_INCLUDE_PATH} ${MPI_INCLUDE_PATH} ${PROJECT_SOURCE_DIR})
target_link_libraries(VovisRenderer ${SSL_LIB} ${OSPRAY_LIB} ${OSPRAY_LIB_COMMON} ${OSPRAY_LIB_MPI} ${EMBREE_LIB} ${TBB_LIB} ${MPI_LIB} ${Z_LIB} ${CRYPTO_LIB})