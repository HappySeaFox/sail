cmake_minimum_required(VERSION 3.10)

project(SAIL VERSION 0.9.0
             DESCRIPTION "Squirrel Abstract Image Library"
             LANGUAGES C CXX)

# Used only when SAIL_DEV is ON
#
set(SAIL_DEV_VERSION_SUFFIX "pre7")

include(GNUInstallDirs)
include(CheckIncludeFiles)
include(CheckCSourceCompiles)
include(CMakePushCheckState)

# Our own cmake scripts
#
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(sail_plugin)
include(sail_enable_asan)
include(sail_test)
include(sail_check_build_essential)

# Sanity checks
#
string(FIND "${CMAKE_INSTALL_PREFIX}" " " SPACE_INDEX)
if (SPACE_INDEX GREATER_EQUAL 0)
    message(FATAL_ERROR "The current CMAKE_INSTALL_PREFIX contains spaces: '${CMAKE_INSTALL_PREFIX}'. \
The generated pkg-config files will also have spaces in paths. Pkg-config doesn't support that \
and will fail to generate CFLAGS and LIBS for client applications. Please use a different CMAKE_INSTALL_PREFIX \
without spaces.")
endif()

# Check for required includes
#
sail_check_include(ctype.h)
sail_check_include(errno.h)
sail_check_include(setjmp.h)
sail_check_include(stdarg.h)
sail_check_include(stdbool.h)
sail_check_include(stddef.h)
sail_check_include(stdint.h)
sail_check_include(stdio.h)
sail_check_include(stdlib.h)
sail_check_include(string.h)
sail_check_include(wchar.h)

if (UNIX)
    sail_check_include(dirent.h)
    sail_check_include(dlfcn.h)
    sail_check_include(sys/stat.h)
    sail_check_include(sys/time.h)
    sail_check_include(sys/types.h)
    sail_check_include(unistd.h)
endif()

if (WIN32)
    sail_check_include(io.h)
    sail_check_include(share.h)
    sail_check_include(windows.h)
    sail_check_include("windows.h;versionhelpers.h")
endif()

# Options
#
option(SAIL_DEV "Enable developer mode. Be more strict when compiling source code, for example." OFF)
set(SAIL_ONLY_PLUGINS "" CACHE STRING "Enable only the plugins specified in this ';'-separated list. \
Plugins with missing dependencies will be disabled regardless this setting")
set(SAIL_EXCEPT_PLUGINS "" CACHE STRING "Enable all plugins except the plugins specified in this ';'-separated list. \
Plugins with missing dependencies will be disabled regardless this setting")
option(SAIL_READ_OUTPUT_BPP32_BGRA "Make the read operations output BPP32-BGRA pixels instead of BPP32-RGBA." OFF)

if (SAIL_READ_OUTPUT_BPP32_BGRA)
    set(SAIL_DEFAULT_READ_OUTPUT_PIXEL_FORMAT "BPP32-BGRA")
else()
    set(SAIL_DEFAULT_READ_OUTPUT_PIXEL_FORMAT "BPP32-RGBA")
endif()

set(SAIL_MAGIC_BUFFER_SIZE 10)

# TODO Move to a Debian patch
# Set arch-specific include/lib dirs
#
#set(CMAKE_INSTALL_INCLUDEDIR "include/${CMAKE_LIBRARY_ARCHITECTURE}")
#set(CMAKE_INSTALL_LIBDIR "lib/${CMAKE_LIBRARY_ARCHITECTURE}")

# Enable shared libs by default
#
set(BUILD_SHARED_LIBS ON)

# Enable strict C11
#
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)

# Enable strict C++11
#
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Internal flag used to include SAIL headers locally with "header.h" or <sail/header.h> otherwise
#
add_definitions(-DSAIL_BUILD)

if (SAIL_DEV)
    add_definitions(-DSAIL_DEV)
endif()

# Enable as many warnings as possible
#
if (MSVC)
    if (CMAKE_C_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

    if (SAIL_DEV)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
    endif()
endif()

# Thread local flag used with static variables
#
if (MSVC)
    set(SAIL_THREAD_LOCAL "__declspec(thread)")
else()
    set(SAIL_THREAD_LOCAL "_Thread_local")
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
endif()

if (UNIX)
    set(SAIL_UNIX ON)
endif()

if (MINGW)
    set(SAIL_MINGW ON)
endif()

if (CYGWIN)
    set(SAIL_CYGWIN ON)
endif()

if (APPLE)
    set(SAIL_APPLE ON)
endif()

# Common required packages
#
if (WIN32)
    set(PKG_CONFIG_EXECUTABLE "${PROJECT_SOURCE_DIR}/extra/bin/pkg-config.exe")
    set(ENV{PKG_CONFIG_PATH} "${PROJECT_SOURCE_DIR}/extra/B/lib/pkgconfig")
endif()

find_package(PkgConfig REQUIRED)

# Common configuration file
#
set(SAIL_PLUGINS_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/sail/plugins")
if (WIN32)
    string(REPLACE "/" "\\\\" SAIL_PLUGINS_PATH "${SAIL_PLUGINS_PATH}")
endif()
set(SAIL_ICONS_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/sail/icons")
if (WIN32)
    string(REPLACE "/" "\\\\" SAIL_ICONS_PATH "${SAIL_ICONS_PATH}")
endif()

configure_file("${PROJECT_SOURCE_DIR}/src/config.h.in" "${PROJECT_BINARY_DIR}/include/config.h" @ONLY)
include_directories("${PROJECT_BINARY_DIR}/include")

# Version suffix for pre-release versions used in dist utils to build installers
#
if (SAIL_DEV)
    set(SAIL_DIST_VERSION_SUFFIX "-${SAIL_DEV_VERSION_SUFFIX}")
endif()

# INNO Setup
#
configure_file("${PROJECT_SOURCE_DIR}/LICENSE.txt" "${PROJECT_BINARY_DIR}/LICENSE.txt" COPYONLY)
configure_file("${PROJECT_SOURCE_DIR}/dist/win/sail.iss" "${PROJECT_BINARY_DIR}/sail.iss" @ONLY)

# Configure subdirs
#

find_package(SDL2)
set(SAIL_SDL_EXAMPLE OFF)

add_subdirectory(src/libsail-common)
add_subdirectory(src/libsail)
add_subdirectory(src/sail-plugins)
add_subdirectory(src/bindings/c++)
add_subdirectory(examples/c/sail-convert)
add_subdirectory(examples/c/sail-probe)
if (SDL2_FOUND)
    set(SAIL_SDL_EXAMPLE ON)
    add_subdirectory(examples/c/sail-sdl-viewer)
endif()
add_subdirectory(tests)

# Installation
#
install(FILES "${PROJECT_BINARY_DIR}/include/config.h" DESTINATION include/sail/sail-common)

if (WIN32)
    if (EXISTS "${PROJECT_SOURCE_DIR}/extra/B/bin")
        install(DIRECTORY "${PROJECT_SOURCE_DIR}/extra/B/bin/" DESTINATION lib/sail/plugins/lib)
    endif()
else()
    if (EXISTS "${PROJECT_SOURCE_DIR}/extra/B/lib")
        install(DIRECTORY "${PROJECT_SOURCE_DIR}/extra/B/lib/" DESTINATION lib/sail/plugins/lib)
    endif()
endif()

install(DIRECTORY "${PROJECT_SOURCE_DIR}/icons/" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/sail/icons")

# Print configuration statistics
#
if (SAIL_COLORED_OUTPUT)
    set(SAIL_COLORED_OUTPUT_CLARIFY " (on Windows >= 10 and Unix)")
endif()
message("")
message("***************************************")
message("*")
message("* Configuration statistics: ")
message("*")
message("* SAIL version:                 ${PROJECT_VERSION}")
message("* Developer mode:               ${SAIL_DEV}")
message("* SDL example:                  ${SAIL_SDL_EXAMPLE}")
message("*")
message("* Install prefix:               ${CMAKE_INSTALL_PREFIX}")
message("* LIBDIR:                       ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
message("* INCLUDEDIR:                   ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
message("* DATADIR:                      ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}")
message("*")
message("* Colored output:               ${SAIL_COLORED_OUTPUT}${SAIL_COLORED_OUTPUT_CLARIFY}")
message("* Enabled plugins:              ${PLUGINS}")
message("* Enabled conditional plugins:  ${ENABLED_CONDITIONAL_PLUGINS}")
message("* Disabled conditional plugins: ${DISABLED_CONDITIONAL_PLUGINS}")
message("* Default read output format:   ${SAIL_DEFAULT_READ_OUTPUT_PIXEL_FORMAT}")
message("*")
message("***************************************")
message("")
