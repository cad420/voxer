find_package(Threads REQUIRED)
find_package(OpenCV 4.0.0 REQUIRED COMPONENTS highgui)

add_executable(TestAnnotationGrabCut Annotation/grabcut.cpp)
target_link_libraries(
    TestAnnotationGrabCut
    PRIVATE
    Threads::Threads
    voxer_core
    ${OpenCV_LIBS}
)
target_compile_features(TestAnnotationGrabCut PRIVATE cxx_std_14)

add_executable(TestAnnotationLevelSet Annotation/levelset.cpp)
target_link_libraries(
    TestAnnotationLevelSet
    PRIVATE
    Threads::Threads
    voxer_core
    ${OpenCV_LIBS}
)
target_compile_features(TestAnnotationLevelSet PRIVATE cxx_std_14)

enable_testing()
add_test(NAME TestAnnotationGrabCut COMMAND TestAnnotationGrabCut)
add_test(NAME TestAnnotationLevelSet COMMAND TestAnnotationLevelSet)

