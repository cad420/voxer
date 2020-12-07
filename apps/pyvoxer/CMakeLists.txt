find_program(PYTHON python)
set(PYTHON_EXECUTABLE ${PYTHON} CACHE STRING "Python path")

include(${PROJECT_SOURCE_DIR}/third_party/pybind11.cmake)

pybind11_add_module(pyvoxer pyvoxer.cpp)
target_link_libraries(pyvoxer PRIVATE voxer::voxer_core)
target_compile_definitions(pyvoxer PRIVATE VERSION_INFO="${PROJECT_VERSION}")
set_target_properties(pyvoxer PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
# TODO: set LIBRARY_OUTPUT_DIRECTORY_DEBUG or _RELEASE if in Windows