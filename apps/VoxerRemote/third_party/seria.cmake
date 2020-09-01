include(FetchContent)
FetchContent_Declare(
    seria
    GIT_REPOSITORY https://github.com/ukabuer/seria.git
    GIT_TAG 520eec57869741a6674c34a4a0275b308483b063
    GIT_SHALLOW true
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(seria)