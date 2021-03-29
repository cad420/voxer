find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)
include(${PROJECT_SOURCE_DIR}/third_party/fmt.cmake)
include(${PROJECT_SOURCE_DIR}/third_party/spdlog.cmake)
#include(${PROJECT_SOURCE_DIR}/third_party/CImg.cmake)

list(
    APPEND
    VOXER_CORE_SRCS
    Data/Annotation.cpp
    Data/Color.cpp
    Data/Image.cpp
    Data/StructuredGrid.cpp
    Data/TransferFunction.cpp
    Mappers/StructuredGridHistogramMapper.cpp
    Filters/BlurFilter.cpp
    Filters/EqualizeFilter.cpp
    Filters/ContrastFilter.cpp
    Filters/GradientFilter.cpp
    Renderers/VolumeRenderer.cpp
    Renderers/SliceRenderer.cpp
    IO/MRC/MRCReader.cpp
    IO/Raw/RawReader.cpp
)

if (VOXER_BUILD_ANNOTATION_MODULE)
  find_package(OpenCV 4.0.0 REQUIRED COMPONENTS core imgproc cudaimgproc)
  list(
      APPEND
      VOXER_CORE_SRCS
      ${CMAKE_CURRENT_SOURCE_DIR}/Filters/AnnotationLevelSetFilter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Filters/AnnotationGrabCutFilter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Filters/seg_levelset.cpp
  )
endif ()

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
    ${PROJECT_SOURCE_DIR}/third_party/
    ${PROJECT_SOURCE_DIR}/third_party/glad/include/
)
target_link_libraries(
    voxer_core
    PRIVATE
    spdlog
    PUBLIC
    Threads::Threads
    fmt::fmt
    spdlog
    OpenMP::OpenMP_CXX
    ${CMAKE_DL_LIBS}
)

if (VOXER_BUILD_ANNOTATION_MODULE)
  target_link_libraries(
      voxer_core
      PUBLIC
      ${OpenCV_LIBS}
  )
endif ()

target_compile_features(voxer_core PRIVATE cxx_std_14)
target_compile_definitions(voxer_core PRIVATE cimg_display=0)
if (UNIX)
  target_compile_options(voxer_core PRIVATE -Wall -Wextra)
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

if (VOXER_BUILD_RENDERER_OSPRAY)
  add_subdirectory(Renderers/OSPRay)
endif ()

if (VOXER_BUILD_RENDERER_OPENGL)
  add_subdirectory(Renderers/OpenGL)
endif ()

if (VOXER_INSTALL)
  install(
      TARGETS voxer_core fmt spdlog
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
