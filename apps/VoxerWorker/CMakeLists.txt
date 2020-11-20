cmake_minimum_required(VERSION 3.11)

project(VoxerWorker LANGUAGES CXX C VERSION 0.1.0)

option(VOXER_BUILD_ANNOTATION_MODULE "build annotation related module" ON)

include(${PROJECT_SOURCE_DIR}/third_party/seria.cmake)
include(${PROJECT_SOURCE_DIR}/third_party/poco.cmake)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

list(
    APPEND
    VoxerWorker_SRCS
    main.cpp
    VoxerWorkerApplication.cpp
    Server.cpp
    Service/AbstractService.cpp
    Service/JSONRPCService.cpp
    Service/DatasetService.cpp
    Service/SliceService.cpp
    Service/VolumeRenderingService.cpp
    Store/DatasetStore.cpp
    ManagerAPI/dataset.cpp
    JSONRPC/RPCMethodsStore.cpp
)

if (VOXER_BUILD_ANNOTATION_MODULE)
  list(
      APPEND
      VoxerWorker_SRCS
      Service/AnnotationService.cpp
  )
endif ()

add_executable(VoxerWorker ${VoxerWorker_SRCS})
target_include_directories(
    VoxerWorker
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    VoxerWorker
    PRIVATE
    voxer::voxer_core
    seria::seria
    fmt::fmt
    ZLIB::ZLIB
    Threads::Threads
    Poco::Net
    Poco::Util
)
target_compile_features(VoxerWorker PRIVATE cxx_std_17)
if (VOXER_BUILD_ANNOTATION_MODULE)
  target_compile_definitions(VoxerWorker PRIVATE ENABLE_ANNOTATION_SERVICE)
endif ()

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
install(
    TARGETS VoxerWorker
    DESTINATION bin
)
