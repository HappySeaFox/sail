cmake_minimum_required(VERSION 3.2)

project(SAIL VERSION 0.9.0
             DESCRIPTION "Squirrel Abstract Image Libraries"
             LANGUAGES C)

# Enable shared libs by default
#
set(BUILD_SHARED_LIBS ON)

# Enable strict C99
#
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)

# Internal flag used to export symbols
#
add_definitions(-DSAIL_BUILD)

# Enable more warnings
#
if (CMAKE_COMPILER_IS_GNUCC)
    add_compile_options(-Wall -Wextra)
elseif (MSVC)
    add_compile_options(/W4)
endif()

# Platform definitions used in config.h
#
if (WIN32)
    set(SAIL_WIN32 ON)
elseif (UNIX)
    set(SAIL_UNIX ON)
elseif (MINGW)
    set(SAIL_MINGW ON)
elseif(CYGWIN)
    set(SAIL_CYGWIN ON)
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
