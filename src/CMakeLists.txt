find_package(ospray REQUIRED)
# find_package(MPI REQUIRED)
find_package(Threads REQUIRED)
find_package(glfw3 REQUIRED)

vm_external_module(GIT_REPOSITORY https://github.com/cad420/VMCore.git GIT_TAG master)

# TODO: split ospray and opengl implmentation

list(
    APPEND
    VOXER_SRCS
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

if (VOXER_ENABLE_GPU)
  list(
      APPPEND
      VOXER_SRCS
      ${CMAKE_CURRENT_SOURCE_DIR}/VTKRenderer.cpp
  )
endif ()

add_library(voxer STATIC ${VOXER_SRCS})
target_include_directories(
    voxer
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third_party/rapidjson/include/>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/third_party/seria/include/
    ${PROJECT_SOURCE_DIR}/third_party/fmt/include/
    ${PROJECT_SOURCE_DIR}/third_party/glad/include/
)
target_link_libraries(
    voxer
    PUBLIC Threads::Threads ospray::ospray glfw
    PRIVATE vmcore
)

if (VOXER_USE_GPU)
  find_package(VTK REQUIRED COMPONENTS vtkRenderingVolumeOpenGL2 vtkIOImage)
  include(${VTK_USE_FILE})
  target_link_libraries(voxer PRIVATE ${VTK_LIBRARIES})
endif ()

target_compile_options(voxer PRIVATE -Werror)
target_compile_features(voxer PRIVATE cxx_std_14)

#install(
#    TARGETS voxer
#    DESTINATION lib
#    EXPORT voxer-target
#)
#install(
#    DIRECTORY ${PROJECT_SOURCE_DIR}/include/voxer
#    DESTINATION include
#)
#install(
#    EXPORT voxer-target
#    FILE VoxerTargets.cmake
#    NAMESPACE voxer::
#    DESTINATION lib/cmake/voxer
#)
#
#include(CMakePackageConfigHelpers)
#configure_package_config_file(
#    ${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in
#    "${CMAKE_CURRENT_BINARY_DIR}/VoxerConfig.cmake"
#    INSTALL_DESTINATION "lib/cmake/voxer"
#    NO_SET_AND_CHECK_MACRO
#    NO_CHECK_REQUIRED_COMPONENTS_MACRO
#)
#write_basic_package_version_file(
#    "${CMAKE_CURRENT_BINARY_DIR}/VoxerConfigVersion.cmake"
#    VERSION ${PACKAGE_VERSION}
#    COMPATIBILITY AnyNewerVersion
#)
#install(
#    FILES
#    ${CMAKE_CURRENT_BINARY_DIR}/VoxerConfig.cmake
#    ${CMAKE_CURRENT_BINARY_DIR}/VoxerConfigVersion.cmake
#    DESTINATION lib/cmake/voxer
#)
#export(
#    EXPORT voxer-target
#    NAMESPACE voxer::
#    FILE "${CMAKE_CURRENT_BINARY_DIR}/VoxerTargets.cmake"
#)w