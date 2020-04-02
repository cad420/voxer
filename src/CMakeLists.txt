find_package(ospray REQUIRED)
# find_package(MPI REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL)

include(${PROJECT_SOURCE_DIR}/third_party/fmt.cmake)
include(${PROJECT_SOURCE_DIR}/third_party/seria.cmake)
include(${PROJECT_SOURCE_DIR}/third_party/vmcore.cmake)

file(
    READ
    Rendering/OpenGL/shaders/position_f.glsl
    POSITION_FRAGMENT_SHADER
)
file(
    READ
    Rendering/OpenGL/shaders/position_v.glsl
    POSITION_VERTEX_SHADER
)
file(
    READ
    Rendering/OpenGL/shaders/screenquad_f.glsl
    SCREENQUAD_FRAGMENT_SHADER
)
file(
    READ
    Rendering/OpenGL/shaders/screenquad_v.glsl
    SCREENQUAD_VERTEX_SHADER
)
file(
    READ
    Rendering/OpenGL/shaders/naiveraycast_f.glsl
    NAIVERAYCAST_FRAGMENT_SHADER
)
set_property(
    DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "Rendering/OpenGL/shaders/naiveraycast_f.glsl;Rendering/OpenGL/shaders/screenquad_v.glsl;Rendering/OpenGL/shaders/screenquad_f.glsl;Rendering/OpenGL/shaders/position_f.glsl;Rendering/OpenGL/shaders/position_f.glsl"
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Rendering/OpenGL/shaders.hpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/Rendering/OpenGL/shaders.hpp
    ESCAPE_QUOTES
    NEWLINE_STYLE LF
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

target_compile_options(voxer PRIVATE -Werror)
target_compile_features(voxer PRIVATE cxx_std_14)

get_target_property(OSPRAY_LOCATION ospray::ospray LOCATION)
message(STATUS ${OSPRAY_LOCATION})

install(
    TARGETS voxer
    DESTINATION lib
    EXPORT voxer-targets
)
install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/voxer
    DESTINATION include
)
#install(
#    EXPORT voxer-targets
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
#    EXPORT voxer-targets
#    NAMESPACE voxer::
#    FILE "${CMAKE_CURRENT_BINARY_DIR}/VoxerTargets.cmake"
#)