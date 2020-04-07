cmake_minimum_required(VERSION 3.6)

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

# Enable as many warnings as possible
#
if(MSVC)
    if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    endif()
elseif(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")
endif()

# Disable undefined symbols
#
if (CMAKE_COMPILER_IS_GNUCC)
    if (BUILD_SHARED_LIBS)
        set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
    else()
        set(CMAKE_STATIC_LINKER_FLAGS "-Wl,--no-undefined")
    endif()

    set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-undefined")
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
if (WIN32)
    set(PKG_CONFIG_EXECUTABLE "${PROJECT_SOURCE_DIR}/extra/pkg-config.exe")
    set(ENV{PKG_CONFIG_PATH} "${PROJECT_SOURCE_DIR}/extra/B/lib/pkgconfig")
endif()

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
