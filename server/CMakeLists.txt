cmake_minimum_required(VERSION 3.12)

add_executable(VoxerServer)
target_sources(VoxerServer
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Application.cpp
)
target_include_directories(VoxerServer
  PRIVATE
    ${OSPRAY_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/third_party/
)
target_link_libraries(VoxerServer
  PRIVATE voxer
)
target_compile_features(VoxerServer PRIVATE cxx_std_11)
