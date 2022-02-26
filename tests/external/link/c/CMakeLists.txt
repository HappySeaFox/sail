cmake_minimum_required(VERSION 3.10)

project(external-c-api-link)

include(CheckCSourceRuns)
include(CMakePushCheckState)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../../../../cmake" "${CMAKE_MODULE_PATH}")
include(sail_enable_asan)

add_executable(external-c-api-link main.c)

sail_enable_asan(TARGET external-c-api-link)

# Add SAIL dependency
#
find_package(Sail REQUIRED)
target_link_libraries(external-c-api-link PRIVATE SAIL::sail)
