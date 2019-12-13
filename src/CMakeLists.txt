find_package(ospray REQUIRED)
find_package(MPI REQUIRED)
find_package(simdjson CONFIG REQUIRED)

add_library(voxer STATIC)

list(
    APPEND
    VOXER_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/OSPRayRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SceneDataset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Volume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TransferFunction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Isosurface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Slice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/Debugger.cpp
)

if (VOXER_ENABLE_GPU)
  list(
      APPPEND
      VOXER_SRCS
      ${CMAKE_CURRENT_SOURCE_DIR}/VTKRenderer.cpp
  )
endif ()

target_sources(voxer PRIVATE ${VOXER_SRCS})
target_include_directories(
    voxer
    PUBLIC
    ${OSPRAY_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    PRIVATE
    ${MPI_CXX_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
)
target_link_libraries(
    voxer
    PUBLIC
    ${OSPRAY_LIBRARY}
    simdjson::simdjson
    PRIVATE
    ${MPI_CXX_LIBRARIES}
)

if (VOXER_USE_GPU)
  find_package(VTK REQUIRED COMPONENTS vtkRenderingVolumeOpenGL2 vtkIOImage)
  include(${VTK_USE_FILE})
  target_link_libraries(voxer PRIVATE ${VTK_LIBRARIES})
endif ()

target_compile_features(voxer PRIVATE cxx_std_14)
