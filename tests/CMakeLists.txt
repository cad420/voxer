add_executable(Test)
target_sources(Test
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/DatasetManager.test.cpp
        ${PROJECT_SOURCE_DIR}/apps/server/DatasetManager.cpp
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