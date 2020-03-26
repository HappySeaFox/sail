cmake_minimum_required(VERSION 3.2)

project(SAIL VERSION 1.0.0
             DESCRIPTION "Squirrel Abstract Image Libraries"
             LANGUAGES C)

# Common setup
#
set(BUILD_SHARED_LIBS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)

# Platform definitions used in config.h
#
if(WIN32)
    set(SAIL_PLATFORM_WIN32 ON)
elseif(UNIX)
    set(SAIL_PLATFORM_UNIX ON)
endif()

# Common required packages
#
find_package(PkgConfig REQUIRED)

# Common configuration file
#
configure_file("${PROJECT_SOURCE_DIR}/config.h.in" "${PROJECT_BINARY_DIR}/include/config.h")
include_directories("${PROJECT_BINARY_DIR}/include")

# Configure subdirs
#
add_subdirectory(src/libsail-common)
#add_subdirectory(src/libsail)
add_subdirectory(src/sail-plugins)
