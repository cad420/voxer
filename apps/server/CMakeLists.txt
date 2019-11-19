find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_library(
  USOCKET_LIBRARY
  NAMES uSockets
  HINTS ${USOCKETS_DIR}/lib
)
find_path(USOCKET_INCLUDEP_DIR
  NAMES libusockets.h
  HINTS ${USOCKETS_DIR}/include
)
find_path(UWEBSOCKET_INCLUDE_DIR
  NAMES uwebsockets/WebSocket.h
  HINTS ${USOCKETS_DIR}/include
)

add_executable(voxer-server)
target_sources(voxer-server
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_include_directories(voxer-server
  PRIVATE
    ${USOCKET_INCLUDEP_DIR}
    ${UWEBSOCKET_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(voxer-server
  PRIVATE
    voxer
    Threads::Threads
    ZLIB::ZLIB
    ${USOCKET_LIBRARY}
)
target_compile_features(voxer-server PRIVATE cxx_std_17)
