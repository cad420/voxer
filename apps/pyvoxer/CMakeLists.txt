cmake_minimum_required(VERSION 3.11)

project(pyvoxer LANGUAGES CXX C VERSION 0.1.0)

find_package(seria REQUIRED)
find_package(Voxer REQUIRED)
find_package(ospray 2.0.0 REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL)

include(${PROJECT_SOURCE_DIR}/../../third_party/pybind11.cmake)

pybind11_add_module(pyvoxer pyvoxer.cpp)
target_link_libraries(pyvoxer PRIVATE voxer::voxer)