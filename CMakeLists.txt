cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(BoardGameAI CXX)

# ========== Compiler & linker options ==========
# Enable the highest level of warning
set(COMPILE_OPTIONS
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>: -Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:                 /Wall                   >
)
# Enable code coverage analysis
set(COVERAGE_OPTIONS
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>: -coverage -fprofile-abs-path>
)

# ========== Third-party libraries ==========
# pthread
find_package(Threads REQUIRED)
list(APPEND ThirdPartyLibraries Threads::Threads)
# nlohmann/json
find_package(nlohmann_json CONFIG REQUIRED)
list(APPEND ThirdPartyLibraries nlohmann_json nlohmann_json::nlohmann_json)
# pboettch/json-schema-validator
find_package(nlohmann_json_schema_validator REQUIRED)
list(APPEND ThirdPartyLibraries nlohmann_json_schema_validator)
# gcem
find_package(gcem CONFIG REQUIRED)
list(APPEND ThirdPartyLibraries gcem)
# oneTBB
find_package(TBB CONFIG REQUIRED)
list(APPEND ThirdPartyLibraries TBB::tbb)
# google/test (only for BoardGameAITest)
find_package(GTest CONFIG REQUIRED)

# ========== Targets ==========
# Core library
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM SRC_FILES ${CMAKE_SOURCE_DIR}/src/Main.cpp)
add_library(Core STATIC EXCLUDE_FROM_ALL ${SRC_FILES})
target_compile_options(Core PRIVATE ${COMPILE_OPTIONS})
target_link_libraries(Core PRIVATE ${ThirdPartyLibraries})

# Main executable
add_executable(BoardGameAI ${CMAKE_SOURCE_DIR}/src/Main.cpp)
target_compile_options(BoardGameAI PRIVATE ${COMPILE_OPTIONS})
target_link_libraries(BoardGameAI PRIVATE Core ${ThirdPartyLibraries})

# Tests
file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS test/*.cpp)
add_executable(BoardGameAITest ${TEST_FILES})
target_compile_options(BoardGameAITest PRIVATE ${COMPILE_OPTIONS} ${COVERAGE_OPTIONS})
target_link_options(BoardGameAITest PRIVATE ${COVERAGE_OPTIONS})
target_link_libraries(BoardGameAITest PRIVATE Core GTest::gtest_main ${ThirdPartyLibraries})

# ========== Copy files ==========
# schema
add_custom_command(TARGET Core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/schema/
    ${CMAKE_BINARY_DIR}/schema/
)
