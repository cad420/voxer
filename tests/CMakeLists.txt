add_executable(Test DatasetStore.test.cpp CommandParser.test.cpp)
target_sources(Test
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/DatasetStore.test.cpp
        ${PROJECT_SOURCE_DIR}/apps/server/DatasetStore.cpp
        )

find_package(simdjson CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(fmt CONFIG REQUIRED)

target_include_directories(Test
        PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/server
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}
        )
target_link_libraries(Test
        PRIVATE
        Threads::Threads
        simdjson::simdjson
        fmt::fmt
        )
target_compile_features(Test PRIVATE cxx_std_17)

add_executable(Test2 HexColorToFloat.test.cpp)
target_sources(Test2
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/HexColorToFloat.test.cpp
        )
target_include_directories(Test2
        PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/server
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}
        )
target_link_libraries(Test2
        PRIVATE
        Threads::Threads
        simdjson::simdjson
        fmt::fmt
        )
target_compile_features(Test2 PRIVATE cxx_std_17)


add_executable(Test3 CommandParser.test.cpp)
target_sources(Test3
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/CommandParser.test.cpp
        ${PROJECT_SOURCE_DIR}/apps/server/CommandParser.cpp
        ${PROJECT_SOURCE_DIR}/apps/server/DatasetStore.cpp
        )
target_include_directories(Test3
        PRIVATE
        ${PROJECT_SOURCE_DIR}/apps/server
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}
        )
target_link_libraries(Test3
        PRIVATE
        Threads::Threads
        simdjson::simdjson
        fmt::fmt
        )
target_compile_features(Test3 PRIVATE cxx_std_17)
