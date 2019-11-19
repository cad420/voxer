add_executable(Test)
target_sources(Test
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
)
target_include_directories(Test
  PRIVATE
    ${OSPRAY_INCLUDE_DIRS}
    ${MPI_CXX_INCLUDE_PATH}
    ${PROJECT_SOURCE_DIR}
)
target_link_libraries(Test
  PRIVATE voxer
)
target_compile_features(Test PRIVATE cxx_std_11)