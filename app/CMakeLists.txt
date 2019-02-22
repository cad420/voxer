add_executable(VovisRenderer
  ${CMAKE_CURRENT_SOURCE_DIR}/Application.cpp
)
target_include_directories(VovisRenderer
  PRIVATE
    ${OSPRAY_INCLUDE_DIRS}
    ${MPI_CXX_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(VovisRenderer
  PRIVATE
    VOVIS_LIB
)
target_compile_features(VovisRenderer PRIVATE cxx_std_11)
