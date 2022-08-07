cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(BoardGameAI CXX)

# ========== Compiler & linker options ==========
# Enable the highest level of warning
set(WARNING_OPTIONS
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>: -Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:                 /Wall                   >
)
# Enable code coverage analysis
set(COVERAGE_OPTIONS
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>: -coverage -fprofile-abs-path>
)
# Enable profiling
set(PROFILE_OPTIONS
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>: -pg>
)

# ========== Third-party libraries ==========
# pthread
find_package(Threads REQUIRED)
list(APPEND THRID_PARTY_LIBRARIES Threads::Threads)
# nlohmann/json
find_package(nlohmann_json CONFIG REQUIRED)
list(APPEND THRID_PARTY_LIBRARIES nlohmann_json nlohmann_json::nlohmann_json)
# pboettch/json-schema-validator
find_package(nlohmann_json_schema_validator REQUIRED)
list(APPEND THRID_PARTY_LIBRARIES nlohmann_json_schema_validator)
# gcem
find_package(gcem CONFIG REQUIRED)
list(APPEND THRID_PARTY_LIBRARIES gcem)
# oneTBB
find_package(TBB CONFIG REQUIRED)
list(APPEND THRID_PARTY_LIBRARIES TBB::tbb)

# ========== Third-party libraries for test ==========
# google/test
find_package(GTest CONFIG REQUIRED)
list(APPEND THRID_PARTY_LIBRARIES_FOR_TEST GTest::gtest GTest::gtest_main)

# ========== Third-party libraries for benchmark ==========
# google/benchmark
find_package(benchmark CONFIG REQUIRED)
list(APPEND THRID_PARTY_LIBRARIES_FOR_BENCHMARK benchmark::benchmark benchmark::benchmark_main)

# ========== Targets ==========
# Main executable
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})
list(REMOVE_ITEM SRC_FILES ${CMAKE_SOURCE_DIR}/src/Main.cpp)
target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_OPTIONS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${THRID_PARTY_LIBRARIES})

# Test
file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/test/*.cpp)
add_executable(${PROJECT_NAME}Test ${SRC_FILES} ${TEST_FILES})
target_compile_options(${PROJECT_NAME}Test PRIVATE ${WARNING_OPTIONS} ${COVERAGE_OPTIONS})
target_link_options(${PROJECT_NAME}Test PRIVATE ${COVERAGE_OPTIONS})
target_link_libraries(${PROJECT_NAME}Test PRIVATE ${THRID_PARTY_LIBRARIES} ${THRID_PARTY_LIBRARIES_FOR_TEST})

# Benchmark
file(GLOB_RECURSE BENCHMARK_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/benchmark/*.cpp)
add_executable(${PROJECT_NAME}Benchmark ${SRC_FILES} ${BENCHMARK_FILES})
target_compile_options(${PROJECT_NAME}Benchmark PRIVATE ${WARNING_OPTIONS} ${PROFILE_OPTIONS})
target_link_options(${PROJECT_NAME}Benchmark PRIVATE ${PROFILE_OPTIONS})
target_link_libraries(${PROJECT_NAME}Benchmark PRIVATE ${THRID_PARTY_LIBRARIES} ${THRID_PARTY_LIBRARIES_FOR_BENCHMARK})

# ========== Copy files ==========
# schema
add_custom_command(
    TARGET ${PROJECT_NAME} ${PROJECT_NAME}Test ${PROJECT_NAME}Benchmark
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/schema/
    ${CMAKE_BINARY_DIR}/schema/
)
