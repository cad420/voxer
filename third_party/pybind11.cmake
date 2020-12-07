include(FetchContent)
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11
    GIT_TAG v2.5.0
    GIT_SHALLOW true
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(pybind11)