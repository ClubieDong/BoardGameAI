cmake_minimum_required(VERSION 3.15)

project(BoardGameAI CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/Wall>
)

# Copy schema
file(COPY schema DESTINATION .) # TODO

# pthread
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

# nlohmann/json
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json nlohmann_json::nlohmann_json)

# pboettch/json-schema-validator
find_package(nlohmann_json_schema_validator REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json_schema_validator)

# moodycamel/ConcurrentQueue
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::concurrentqueue::concurrentqueue)
