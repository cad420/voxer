find_package(Threads REQUIRED)
find_package(OpenCV 4.0.0 REQUIRED COMPONENTS highgui)

add_executable(TestAnnotation Annotation/grabcut.cpp)
target_link_libraries(
    TestAnnotation
    PRIVATE
    Threads::Threads
    voxer_core
    ${CMAKE_DL_LIBS}
    ${OpenCV_LIBS}
)
target_compile_features(TestAnnotation PRIVATE cxx_std_14)

enable_testing()
add_test(NAME TestAnnotation COMMAND TestAnnotation)

