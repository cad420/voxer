include(FetchContent)
FetchContent_Declare(
    CImg
    GIT_REPOSITORY https://github.com/dtschump/CImg.git
#    GIT_TAG 2f7a1ebf1b1dcc59d5d2f54c39e291def47b5064
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_GetProperties(CImg)
if (NOT CImg_POPULATED)
  FetchContent_Populate(CImg)
endif ()

FetchContent_GetProperties(
    CImg
    SOURCE_DIR CImg_src
)
add_library(CImg INTERFACE)
target_include_directories(
    CImg
    INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CImg_src}>
)
target_compile_definitions(CImg INTERFACE cimg_display=0)
