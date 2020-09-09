include(FetchContent)
FetchContent_Declare(
    seria
    GIT_REPOSITORY https://github.com/ukabuer/seria.git
    GIT_TAG 644a85083733518d15e653f6629cc11f587d5df6
    GIT_SHALLOW true
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(seria)