# precompiled dependencies
find_package(ospray 2.0.0 REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL)

# dependencies from source
include(${PROJECT_SOURCE_DIR}/third_party/fmt.cmake)
include(${PROJECT_SOURCE_DIR}/third_party/seria.cmake)
include(${PROJECT_SOURCE_DIR}/third_party/vmcore.cmake)

embed_shaders(
    CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Rendering/OpenGL/shaders.hpp.in
    TARGET ${CMAKE_CURRENT_SOURCE_DIR}/Rendering/OpenGL/shaders.hpp
    DIR ${CMAKE_CURRENT_SOURCE_DIR}/Rendering/OpenGL/shaders/
)

# TODO: split ospray and opengl implmentation

list(
    APPEND
    VOXER_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/Dataset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DatasetStore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/databases/MRC/MRCReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/databases/Raw/RawReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scene/Scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scene/SceneDataset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scene/Volume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scene/TransferFunction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scene/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scene/Isosurface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scene/Slice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filter/differ.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filter/histogram.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Rendering/RenderingContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Rendering/OpenGL/RenderingContextOpenGL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Rendering/OSPRay/RenderingContextOSPRay.cpp
    ${PROJECT_SOURCE_DIR}/third_party/glad/src/glad.c
)

add_library(voxer STATIC ${VOXER_SRCS})
target_include_directories(
    voxer
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/third_party/glad/include/
)
target_link_libraries(
    voxer
    PUBLIC Threads::Threads ospray::ospray OpenGL::OpenGL OpenGL::EGL seria::seria fmt::fmt ${CMAKE_DL_LIBS}
    PRIVATE vmcore
)
target_compile_options(voxer PRIVATE -Wall -Wextra)
target_compile_features(voxer PRIVATE cxx_std_14)
if (UNIX)
  set_target_properties(
    voxer
    PROPERTIES
    CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize=leak -fsanitize=undefined"
  )
  set_target_properties(
    voxer
    PROPERTIES
    CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address -fsanitize=leak -fsanitize=undefined"
  )
endif()
add_library(voxer::voxer ALIAS voxer)

option(VOXER_INSTALL "install voxer library and headers" ON)
if (VOXER_INSTALL)
  install(
      TARGETS voxer fmt vmcore seria
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