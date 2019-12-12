add_executable(voxer-server)
target_sources(voxer-server
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CommandParser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DatasetStore.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/PipelineStore.cpp
        )

find_package(simdjson CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_library(
        USOCKET_LIBRARY
        NAMES uSockets
        HINTS ${USOCKETS_DIR}/lib
)
find_path(USOCKET_INCLUDE_DIR
        NAMES libusockets.h
        HINTS ${USOCKETS_DIR}/include
        )
find_path(UWEBSOCKET_INCLUDE_DIR
        NAMES uwebsockets/WebSocket.h
        HINTS ${UWEBSOCKETS_DIR}/include
        )

target_include_directories(voxer-server
        PRIVATE
        ${USOCKET_INCLUDE_DIR}
        ${UWEBSOCKET_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        )
target_link_libraries(voxer-server
        PRIVATE
        voxer
        Threads::Threads
        ZLIB::ZLIB
        ${USOCKET_LIBRARY}
        simdjson::simdjson
        fmt::fmt
        )
target_compile_features(voxer-server PRIVATE cxx_std_17)
