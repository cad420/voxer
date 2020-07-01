include_guard()

option(VMCORE_SHARED_LIBRARY "" OFF)
include(FetchContent)
FetchContent_Declare(
    vmcore
    GIT_REPOSITORY https://github.com/cad420/VMCore.git
    GIT_TAG acf5624d4269feb1333b69b3df67720eab16a1e5
    GIT_SHALLOW true
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(vmcore)
