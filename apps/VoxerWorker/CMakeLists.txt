include(${PROJECT_SOURCE_DIR}/third_party/seria.cmake)
include(${PROJECT_SOURCE_DIR}/third_party/poco.cmake)
include(${PROJECT_SOURCE_DIR}/third_party/spdlog.cmake)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

list(
    APPEND
    VoxerWorker_SRCS
    main.cpp
    VoxerWorkerApplication.cpp
    Server/RequestHandlerFactory.cpp
    Server/WebSockeRequestHandler.cpp
    Store/DatasetStore.cpp
    RPC/RPCMethodsStore.cpp
    RPC/Service.cpp
    RPC/MessageQueue.cpp
    ManagerAPI/ManagerAPIClient.cpp
)

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
    spdlog
    mpack
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
