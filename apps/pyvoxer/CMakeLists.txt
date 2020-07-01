cmake_minimum_required(VERSION 3.11)

project(pyvoxer LANGUAGES CXX C VERSION 0.1.0)

set(IS_MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(IS_MASTER_PROJECT ON)
endif ()

if (IS_MASTER_PROJECT)
  find_package(seria REQUIRED)
  find_package(voxer REQUIRED)
  find_package(ospray 2.0.0 REQUIRED)
  find_package(Threads REQUIRED)
  find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL)
endif ()

find_program(PYTHON python)
set(PYTHON_EXECUTABLE ${PYTHON} CACHE STRING "Python path")

include(${PROJECT_SOURCE_DIR}/third_party/pybind11.cmake)

pybind11_add_module(pyvoxer pyvoxer.cpp)
target_link_libraries(pyvoxer PRIVATE voxer::voxer)
target_compile_definitions(pyvoxer PRIVATE VERSION_INFO="${PROJECT_VERSION}")
set_target_properties(pyvoxer PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
# TODO: set LIBRARY_OUTPUT_DIRECTORY_DEBUG or _RELEASE if in Windows