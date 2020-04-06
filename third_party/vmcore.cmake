include_guard()

option(VMCORE_SHARED_LIBRARY "" OFF)
include(FetchContent)
FetchContent_Declare(
    vmcore
    GIT_REPOSITORY https://github.com/cad420/VMCore.git
    GIT_TAG master
    GIT_SHALLOW true
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(vmcore)
