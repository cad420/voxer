# precompiled dependencies
find_package(Threads REQUIRED)

# dependencies from source
include(${PROJECT_SOURCE_DIR}/third_party/fmt.cmake)
include(${PROJECT_SOURCE_DIR}/third_party/seria.cmake)
include(${PROJECT_SOURCE_DIR}/third_party/vmcore.cmake)

list(
    APPEND
    VOXER_CORE_SRCS
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
)

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
    PUBLIC Threads::Threads seria::seria fmt::fmt ${CMAKE_DL_LIBS}
    PRIVATE vmcore -static-libstdc++
)
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
endif()

if (VOXER_INSTALL)
  install(
      TARGETS voxer_core fmt vmcore seria
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

if (VOXER_BUILD_BACKEND_OSPRAY)
  add_subdirectory(Rendering/OSPRay)
endif ()

if (VOXER_BUILD_BACKEND_GL)
  add_subdirectory(Rendering/OpenGL)
endif ()