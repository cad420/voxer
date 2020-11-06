find_package(ospray 2.0.0 REQUIRED)

list(
    APPEND
    VOXER_BACKEND_OSPRAY_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/OSPRayRenderer.cpp
)

add_library(voxer_backend_ospray SHARED ${VOXER_BACKEND_OSPRAY_SRCS})
target_link_libraries(
    voxer_backend_ospray
    PUBLIC
    voxer::voxer_core
    ospray::ospray
    ${CMAKE_DL_LIBS}
)
target_include_directories(
    voxer_backend_ospray
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)
target_compile_options(voxer_backend_ospray PRIVATE -Wall -Wextra)
target_compile_features(voxer_backend_ospray PRIVATE cxx_std_14)

if (VOXER_INSTALL)
  install(
      TARGETS voxer_backend_ospray
      DESTINATION lib
      EXPORT voxer-targets
  )
endif ()