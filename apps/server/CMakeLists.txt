find_package(fmt CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_library(
    USOCKETS_LIBRARY
    NAMES uSockets
    HINTS ${USOCKETS_LIB_PATH}
)
find_path(
    USOCKETS_INCLUDE_DIR
    NAMES libusockets.h
    HINTS ${USOCKETS_INCLUDE_PATH}
)
find_path(
    UWEBSOCKET_INCLUDE_DIR
    NAMES uwebsockets/WebSocket.h
    HINTS ${UWEBSOCKETS_INCLUDE_PATH}
)

add_executable(
    voxer-server
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CommandParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PipelineStore.cpp
)
target_include_directories(
    voxer-server
    PRIVATE
    ${USOCKETS_INCLUDE_DIR}
    ${UWEBSOCKET_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    voxer-server
    PRIVATE
    voxer
    Threads::Threads
    ZLIB::ZLIB
    ${USOCKETS_LIBRARY}
    fmt::fmt
    fmt::fmt-header-only
)
target_compile_features(voxer-server PRIVATE cxx_std_17)

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
install(
    TARGETS voxer-server
    DESTINATION bin
)
