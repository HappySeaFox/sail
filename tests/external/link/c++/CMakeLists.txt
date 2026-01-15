cmake_minimum_required(VERSION 3.10)

project(external-c++-api-link)

option(SAIL_ASAN "Enable AddressSanitizer." OFF)
option(SAIL_TSAN "Enable ThreadSanitizer." OFF)

# Enable strict C++11
#
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckCSourceRuns)
include(CMakePushCheckState)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../../../../cmake" "${CMAKE_MODULE_PATH}")
include(sail_enable_asan)
include(sail_enable_tsan)

add_executable(external-c++-api-link main.cpp)

# Create and configure common flags interface library
#
add_library(sail-common-flags INTERFACE)
if (SAIL_ASAN)
    sail_enable_asan(INTERFACE_LIB sail-common-flags)
endif()
if (SAIL_TSAN)
    sail_enable_tsan(INTERFACE_LIB sail-common-flags)
endif()

target_link_libraries(external-c++-api-link PRIVATE $<BUILD_INTERFACE:sail-common-flags>)

# Add SAIL dependency
#
find_package(SailC++ REQUIRED)
target_link_libraries(external-c++-api-link PRIVATE SAIL::sail-c++)
