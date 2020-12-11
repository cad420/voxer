find_package(Threads REQUIRED)
include(${PROJECT_SOURCE_DIR}/third_party/catch2.cmake)

if (VOXER_BUILD_ANNOTATION_MODULE)
  find_package(OpenCV 4.0.0 REQUIRED COMPONENTS highgui)
  add_executable(TestAnnotationGrabCut Annotation/grabcut.cpp)
  target_link_libraries(
      TestAnnotationGrabCut
      PRIVATE
      Threads::Threads
      voxer_core
      catch2
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
endif ()

add_executable(MRCReaderTest IO/MRCReader.cpp)
target_link_libraries(MRCReaderTest PRIVATE voxer_core Catch2WithMain)

enable_testing()
if (VOXER_BUILD_ANNOTATION_MODULE)
  add_test(NAME TestAnnotationGrabCut COMMAND TestAnnotationGrabCut)
  add_test(NAME TestAnnotationLevelSet COMMAND TestAnnotationLevelSet)
endif ()
add_test(TestMRCReader COMMAND MRCReaderTest)
