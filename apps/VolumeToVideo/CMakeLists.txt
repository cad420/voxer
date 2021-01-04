aux_source_directory(./src SRC)

add_executable(VolumeToVideo ${SRC})
target_link_libraries(
    VolumeToVideo
    PRIVATE
    voxer::voxer_core
    avcodec
    avutil
    avformat
    swscale
)