include(FetchContent)
FetchContent_Declare(
    seria
    GIT_REPOSITORY https://github.com/ukabuer/seria.git
    GIT_TAG eb6e75d0cb175950a7483ac70717c7f74685a780
    GIT_SHALLOW true
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(seria)