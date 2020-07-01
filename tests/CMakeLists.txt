find_package(Threads REQUIRED)

add_executable(TestDataStore DatasetStore.test.cpp)
target_include_directories(
    TestDataStore
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}
)
target_link_libraries(
    TestDataStore
    PRIVATE
    voxer
    Threads::Threads
)
target_compile_features(TestDataStore PRIVATE cxx_std_17)

add_executable(TestUtils utils.test.cpp)
target_include_directories(
    TestUtils
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}
)
target_link_libraries(
    TestUtils
    PRIVATE
    Threads::Threads
    voxer
)
target_compile_features(TestUtils PRIVATE cxx_std_17)

add_executable(
    TestCommandParser
    ${CMAKE_CURRENT_SOURCE_DIR}/CommandParser.test.cpp
    ${PROJECT_SOURCE_DIR}/apps/VoxerRemote/CommandParser.cpp
)
target_include_directories(
    TestCommandParser
    PRIVATE
    ${PROJECT_SOURCE_DIR}/apps/VoxerRemote
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}
)
target_link_libraries(
    TestCommandParser
    PRIVATE
    voxer
)
target_compile_features(TestCommandParser PRIVATE cxx_std_17)

add_executable(TestServerUtils ServerUtils.test.cpp)
target_include_directories(
    TestServerUtils
    PRIVATE
    ${PROJECT_SOURCE_DIR}/apps/VoxerRemote
    ${PROJECT_SOURCE_DIR}
)
target_link_libraries(
    TestServerUtils
    PRIVATE
    Threads::Threads
)
target_compile_features(TestServerUtils PRIVATE cxx_std_14)

enable_testing()
add_test(NAME TestDataStore COMMAND TestDataStore)
add_test(NAME TestUtils COMMAND TestUtils)
add_test(NAME TestCommandParser COMMAND TestCommandParser)
add_test(NAME TestServerUtils COMMAND TestServerUtils)

