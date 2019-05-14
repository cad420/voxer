add_executable(VoxerServer
  ${CMAKE_CURRENT_SOURCE_DIR}/Application.cpp
)
target_include_directories(VoxerServer
  PRIVATE
    ${OSPRAY_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}
)
target_link_libraries(VoxerServer
  PRIVATE
    voxer
)
target_compile_features(VoxerServer PRIVATE cxx_std_11)
