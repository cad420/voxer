find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)
find_package(ospray 2.0.0 REQUIRED)
include(${PROJECT_SOURCE_DIR}/third_party/fmt.cmake)

list(
    APPEND
    VOXER_CORE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/Data/Annotation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Data/Color.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Data/Image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Data/StructuredGrid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Data/TransferFunction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Filters/differ.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Filters/histogram.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Filters/BlurFilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Filters/EqualizeFilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Filters/ContrastFilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Filters/GradientFilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IO/MRC/MRCReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IO/Raw/RawReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Rendering/VolumeRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Rendering/SliceRenderer.cpp
)

if (VOXER_BUILD_ANNOTATION_MODULE)
  find_package(OpenCV 4.0.0 REQUIRED COMPONENTS core imgproc cudaimgproc)
  list(
    APPEND
    VOXER_CORE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/Filters/AnnotationLevelSetFilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Filters/seg_levelset.cpp
  )
endif()

add_library(voxer_core STATIC ${VOXER_CORE_SRCS})
add_library(voxer::voxer_core ALIAS voxer_core)
target_include_directories(
    voxer_core
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/third_party/glad/include/
)
target_link_libraries(
    voxer_core
    PUBLIC
    Threads::Threads
    fmt::fmt
    OpenMP::OpenMP_CXX
    ${CMAKE_DL_LIBS}
    ospray::ospray
    PRIVATE -static-libstdc++
)

if (VOXER_BUILD_ANNOTATION_MODULE)
  target_link_libraries(
      voxer_core
      PUBLIC
      ${OpenCV_LIBS}
  )
endif()

target_compile_options(voxer_core PRIVATE -Wall -Wextra)
target_compile_features(voxer_core PRIVATE cxx_std_14)
if (UNIX)
  set(SANITIZER "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize=leak -fsanitize=undefined")
  set_target_properties(
      voxer_core
      PROPERTIES
      CMAKE_CXX_FLAGS_DEBUG ${SANITIZER}
  )
  set_target_properties(
      voxer_core
      PROPERTIES
      CMAKE_LINKER_FLAGS_DEBUG ${SANITIZER}
  )
endif ()

if (VOXER_BUILD_BACKEND_OSPRAY)
  add_subdirectory(Rendering/OSPRay)
endif ()

if (VOXER_BUILD_BACKEND_OPENGL)
  add_subdirectory(Rendering/OpenGL)
endif ()

if (VOXER_INSTALL)
  install(
      TARGETS voxer_core fmt
      DESTINATION lib
      EXPORT voxer-targets
  )
  install(
      DIRECTORY ${PROJECT_SOURCE_DIR}/include/voxer
      DESTINATION include
  )
  install(
      EXPORT voxer-targets
      FILE voxer-targets.cmake
      NAMESPACE voxer::
      DESTINATION lib/cmake/voxer
  )

  include(CMakePackageConfigHelpers)
  configure_package_config_file(
      ${PROJECT_SOURCE_DIR}/cmake/config.cmake.in
      "${CMAKE_CURRENT_BINARY_DIR}/voxer-config.cmake"
      INSTALL_DESTINATION "lib/cmake/voxer"
      NO_SET_AND_CHECK_MACRO
      NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )
  write_basic_package_version_file(
      "${CMAKE_CURRENT_BINARY_DIR}/voxer-config-version.cmake"
      VERSION ${PACKAGE_VERSION}
      COMPATIBILITY AnyNewerVersion
  )
  install(
      FILES
      ${CMAKE_CURRENT_BINARY_DIR}/voxer-config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/voxer-config-version.cmake
      DESTINATION lib/cmake/voxer
  )
  export(
      EXPORT voxer-targets
      NAMESPACE voxer::
      FILE "${CMAKE_CURRENT_BINARY_DIR}/voxer-targets.cmake"
  )
endif ()
