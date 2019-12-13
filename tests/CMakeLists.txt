find_package(simdjson CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_executable(TestDataStore)
target_sources(
    TestDataStore
    PRIVATE
    DatasetStore.test.cpp
)
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
    simdjson::simdjson
    fmt::fmt
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
    simdjson::simdjson
    fmt::fmt
)
target_compile_features(TestUtils PRIVATE cxx_std_17)

add_executable(TestCommandParser)
target_sources(
    TestCommandParser
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CommandParser.test.cpp
    ${PROJECT_SOURCE_DIR}/apps/server/CommandParser.cpp
)
target_include_directories(
    TestCommandParser
    PRIVATE
    ${PROJECT_SOURCE_DIR}/apps/server
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}
)
target_link_libraries(
    TestCommandParser
    PRIVATE
    voxer
    fmt::fmt
)
target_compile_features(TestCommandParser PRIVATE cxx_std_17)

add_executable(TestServerUtils ServerUtils.test.cpp)
target_include_directories(
    TestServerUtils
    PRIVATE
    ${PROJECT_SOURCE_DIR}/apps/server
    ${PROJECT_SOURCE_DIR}
)
target_link_libraries(
    TestServerUtils
    PRIVATE
    Threads::Threads
    simdjson::simdjson
    fmt::fmt
)
target_compile_features(TestServerUtils PRIVATE cxx_std_17)
