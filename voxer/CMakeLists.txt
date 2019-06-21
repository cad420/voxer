cmake_minimum_required(VERSION 3.12)

find_package(ospray REQUIRED)
find_package(Poco REQUIRED COMPONENTS Net Util)
find_package(MPI REQUIRED)
find_package(VTK REQUIRED COMPONENTS vtkRenderingVolumeOpenGL2 vtkIOImage)
include(${VTK_USE_FILE})

file(GLOB_RECURSE VOXER_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(voxer STATIC)
target_sources(voxer PRIVATE ${VOXER_SRCS})
target_include_directories(voxer
  PUBLIC
    ${OSPRAY_INCLUDE_DIRS}
  PRIVATE
    ${MPI_CXX_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
)
target_link_libraries(voxer
  PUBLIC
    ${OSPRAY_LIBRARY}
  PRIVATE
    ${VTK_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    ${Poco_LIBRARIES}
)
target_compile_features(voxer PRIVATE cxx_std_11)
