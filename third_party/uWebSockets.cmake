include_guard()

include(FetchContent)
FetchContent_Declare(
    uWebSockets-git
    GIT_REPOSITORY https://github.com/uNetworking/uWebSockets.git
    GIT_TAG v18
)

FetchContent_GetProperties(uWebSockets-git)
if (NOT uWebSockets-git_POPULATED)
  FetchContent_Populate(uWebSockets-git)
endif ()

FetchContent_GetProperties(
    uWebSockets-git
    SOURCE_DIR uWebSockets-git_src
)

add_custom_command(
    OUTPUT ${uWebSockets-git_src}/uSockets/uSockets.a
    COMMAND cp -r src uWebSockets && make
    WORKING_DIRECTORY ${uWebSockets-git_src}
    COMMENT "build uSockets"
    VERBATIM
)
add_custom_target(uSockets DEPENDS ${uWebSockets-git_src}/uSockets/uSockets.a)

add_library(uWebSockets STATIC IMPORTED)
set_property(TARGET uWebSockets PROPERTY IMPORTED_LOCATION ${uWebSockets-git_src}/uSockets/uSockets.a)
set_target_properties(
    uWebSockets PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${uWebSockets-git_src};${uWebSockets-git_src}/uSockets/src"
)
add_dependencies(uWebSockets uSockets)
