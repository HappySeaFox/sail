cmake_minimum_required(VERSION 3.2)

project(SAIL VERSION 1.0.0
             DESCRIPTION "Squirrel Abstract Image Libraries"
             HOMEPAGE_URL "https://github.com/sailor-keg/sail"
             LANGUAGES C)

set(BUILD_SHARED_LIBS ON)
set(CMAKE_C_STANDARD 99)

configure_file("${PROJECT_SOURCE_DIR}/config.h.in" "${PROJECT_BINARY_DIR}/include/config.h")
include_directories("${PROJECT_BINARY_DIR}/include")

add_subdirectory(src/libsail-common)
#add_subdirectory(src/libsail)
#add_subdirectory(src/plugins)
