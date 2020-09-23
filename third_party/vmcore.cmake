include_guard()

option(VMCORE_SHARED_LIBRARY "" OFF)
include(FetchContent)
FetchContent_Declare(
    vmcore
    GIT_REPOSITORY https://github.com/cad420/VMCore.git
    GIT_TAG c25a4b4d3339c03679f9c4cad945e1caa994da04
    GIT_SHALLOW true
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(vmcore)
