add_executable(
  Test
  ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
)
target_include_directories(Test
  PRIVATE
    ${OSPRAY_INCLUDE_DIRS}
    ${MPI_CXX_INCLUDE_PATH}
    ${POCO_INCLUDE_PATH}
    ${PROJECT_SOURCE_DIR}
)
target_link_libraries(Test
  PRIVATE
    VOVIS_LIB
)
target_compile_features(Test PRIVATE cxx_std_11)

add_executable(
  TestSimple
  ${CMAKE_CURRENT_SOURCE_DIR}/1.cpp
)
target_include_directories(TestSimple
  PRIVATE
    ${OSPRAY_INCLUDE_DIRS}
    ${MPI_CXX_INCLUDE_PATH}
    ${PROJECT_SOURCE_DIR}
)
target_link_libraries(TestSimple
  PRIVATE
    ${OSPRAY_LIBRARY}
    ${TBB_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
)
target_compile_features(TestSimple PRIVATE cxx_std_11)
