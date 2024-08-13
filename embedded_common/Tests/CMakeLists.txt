cmake_minimum_required(VERSION 3.8)

set(PROJECT_NAME embedded_common_tests)
project(embedded_common_tests)

set(CMAKE_CXX_STANDARD 17)

# Currently the headers use preprocessor defines to decide
# which peripheral driver headers to include
add_compile_definitions(TESTING)

add_executable(${PROJECT_NAME} tests.c ../Src/helpers.c)

include_directories(
        ${CMAKE_SOURCE_DIR}/../Inc
)

target_link_libraries(${PROJECT_NAME} m) # Link math.h

