include(FetchContent)
FetchContent_Declare(
    seria
    GIT_REPOSITORY https://github.com/ukabuer/seria.git
    GIT_TAG 78452629b800088cd1e434529afdc00a704dd6c7
    GIT_SHALLOW true
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(seria)