cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(BoardGameAI CXX)
set(TEST_PROJECT_NAME ${PROJECT_NAME}Test)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})

list(REMOVE_ITEM SRC_FILES ${CMAKE_SOURCE_DIR}/src/Main.cpp)
file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS test/*.cpp)
add_executable(${TEST_PROJECT_NAME} ${SRC_FILES} ${TEST_FILES})

# Enable the highest level of warning 
set(COMPILE_OPTIONS
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic> 
    $<$<CXX_COMPILER_ID:MSVC>:/Wall>
)
set(COVERAGE_OPTIONS $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:--coverage -fprofile-abs-path>)
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTIONS})
target_compile_options(${TEST_PROJECT_NAME} PRIVATE ${COMPILE_OPTIONS} ${COVERAGE_OPTIONS})
target_link_options(${TEST_PROJECT_NAME} PRIVATE ${COVERAGE_OPTIONS})

# Copy schema
file(COPY schema DESTINATION .) # TODO

# googletest
find_package(GTest CONFIG REQUIRED)
target_link_libraries(${TEST_PROJECT_NAME} PRIVATE GTest::gtest_main)

# pthread
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_link_libraries(${TEST_PROJECT_NAME} PRIVATE Threads::Threads)

# nlohmann/json
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json nlohmann_json::nlohmann_json)
target_link_libraries(${TEST_PROJECT_NAME} PRIVATE nlohmann_json nlohmann_json::nlohmann_json)

# pboettch/json-schema-validator
find_package(nlohmann_json_schema_validator REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json_schema_validator)
target_link_libraries(${TEST_PROJECT_NAME} PRIVATE nlohmann_json_schema_validator)

# gcem
find_package(gcem CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE gcem)
target_link_libraries(${TEST_PROJECT_NAME} PRIVATE gcem)

# Intel's TBB
find_package(TBB CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE TBB::tbb)
target_link_libraries(${TEST_PROJECT_NAME} PRIVATE gcem)
