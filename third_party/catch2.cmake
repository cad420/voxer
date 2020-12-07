# hack to prevent CTest added targets because catch2 has `include(CTest)` in its CMakeList.txt
# see https://stackoverflow.com/a/57240389
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

include(FetchContent)
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.0.0-preview3
    GIT_SHALLOW true
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(catch2)
