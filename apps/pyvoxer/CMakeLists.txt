include(${PROJECT_SOURCE_DIR}/third_party/pybind11.cmake)

pybind11_add_module(pyvoxer pyvoxer.cpp)
target_link_libraries(pyvoxer PRIVATE voxer)