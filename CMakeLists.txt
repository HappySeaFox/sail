cmake_minimum_required(VERSION 3.10)

project(SAIL VERSION 0.9.0
             DESCRIPTION "Squirrel Abstract Image Library"
             LANGUAGES C CXX)

# Used only when SAIL_DEV is ON
#
set(SAIL_DEV_VERSION_SUFFIX "pre10")

include(GNUInstallDirs)
include(CheckIncludeFiles)
include(CheckCSourceCompiles)
include(CMakePushCheckState)

# Our own cmake scripts
#
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
include(sail_check_include)
include(sail_codec)
include(sail_enable_asan)
include(sail_enable_pch)
include(sail_enable_posix_source)
include(sail_install_cmake_config)
include(sail_test)

# Sanity checks
#
string(FIND "${CMAKE_INSTALL_PREFIX}" " " SPACE_INDEX)
if (SPACE_INDEX GREATER_EQUAL 0)
    message(WARNING "The current CMAKE_INSTALL_PREFIX contains spaces: '${CMAKE_INSTALL_PREFIX}'. \
The generated pkg-config files will also have spaces in paths. Pkg-config doesn't support that \
and will fail to generate CFLAGS and LIBS for client applications. Please use CMAKE_INSTALL_PREFIX \
without spaces if you're going to use pkg-config.")
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
sail_check_include(sys/stat.h)
sail_check_include(sys/types.h)
sail_check_include(wchar.h)

if (UNIX)
    sail_check_include(dirent.h)
    sail_check_include(dlfcn.h)
    sail_check_include(sys/time.h)
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
option(SAIL_BUILD_EXAMPLES "Build examples." ON)
option(SAIL_BUILD_TESTS "Build tests." ON)
option(SAIL_DEV "Enable developer mode. Be more strict when compiling source code, for example." OFF)
set(SAIL_EXCEPT_CODECS "" CACHE STRING "Enable all codecs except the codecs specified in this ';'-separated list. \
Codecs with missing dependencies will be disabled regardless this setting")
set(SAIL_ONLY_CODECS "" CACHE STRING "Enable only the codecs specified in this ';'-separated list. \
Codecs with missing dependencies will be disabled regardless this setting")
option(SAIL_READ_OUTPUT_BPP32_BGRA "Make the read operations output BPP32-BGRA pixels instead of BPP32-RGBA." OFF)
option(SAIL_VCPKG_PORT "Build as a VCPKG port. Intended to be used only by VCPKG. When enabled, sets SAIL_VCPKG to ON" OFF)
option(SAIL_VCPKG "Enable VCPKG dependency manager instead of pkg-config on Windows. When enabled, please specify CMAKE_TOOLCHAIN_FILE and CMAKE_BUILD_TYPE. See the vcpkg tutorial for more." OFF)

if (SAIL_VCPKG_PORT)
    set(SAIL_VCPKG ON)
endif()

if (SAIL_VCPKG AND NOT WIN32)
    message(FATAL_ERROR "VCPKG support is Windows-only")
endif()

if (SAIL_READ_OUTPUT_BPP32_BGRA)
    set(SAIL_DEFAULT_READ_OUTPUT_PIXEL_FORMAT "BPP32-BGRA")
else()
    set(SAIL_DEFAULT_READ_OUTPUT_PIXEL_FORMAT "BPP32-RGBA")
endif()

set(SAIL_MAGIC_BUFFER_SIZE 16)

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
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Enable strict C++11
#
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
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
        string(APPEND CMAKE_C_FLAGS " " "/W4")
        string(APPEND CMAKE_CXX_FLAGS " " "/W4")
    endif()
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    string(APPEND CMAKE_C_FLAGS " " "-Wall -Wextra")
    string(APPEND CMAKE_CXX_FLAGS " " "-Wall -Wextra")

    if (SAIL_DEV)
        string(APPEND CMAKE_C_FLAGS " " "-pedantic")
        string(APPEND CMAKE_CXX_FLAGS " " "-pedantic")
    endif()
endif()

# Disable undefined symbols
#
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " " "-Wl,--no-undefined")
    string(APPEND CMAKE_MODULE_LINKER_FLAGS " " "-Wl,--no-undefined")
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " " "-Wl,-undefined,error")
    string(APPEND CMAKE_MODULE_LINKER_FLAGS " " "-Wl,-undefined,error")
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

# pkg-config setup
#
if (NOT SAIL_VCPKG)
    if (WIN32)
        set(PKG_CONFIG_EXECUTABLE "${PROJECT_SOURCE_DIR}/extra/bin/pkg-config.exe")
    endif()

    # Our pkg-config search path to append to the system-wide search path
    #
    # Some development packages (like giflib) have no pkg-config integration. We provide
    # our own pkg-config files for system-wide packages in extra/missing
    #
    if (WIN32)
        set(SAIL_PKGCONFIG_PATH "${PROJECT_SOURCE_DIR}/extra/missing/pkgconfig/${CMAKE_SYSTEM_NAME}\;${PROJECT_SOURCE_DIR}/extra/B/lib/pkgconfig")
    else()
        set(SAIL_PKGCONFIG_PATH "${PROJECT_SOURCE_DIR}/extra/missing/pkgconfig/${CMAKE_SYSTEM_NAME}:${PROJECT_SOURCE_DIR}/extra/B/lib/pkgconfig")
    endif()

    set(ENV_PKG_CONFIG_PATH $ENV{PKG_CONFIG_PATH})
    if (ENV_PKG_CONFIG_PATH)
        if (WIN32)
            set(ENV{PKG_CONFIG_PATH} "${ENV_PKG_CONFIG_PATH}\;${SAIL_PKGCONFIG_PATH}")
        else()
            set(ENV{PKG_CONFIG_PATH} "${ENV_PKG_CONFIG_PATH}:${SAIL_PKGCONFIG_PATH}")
        endif()
    else()
        set(ENV{PKG_CONFIG_PATH} ${SAIL_PKGCONFIG_PATH})
    endif()

    find_package(PkgConfig REQUIRED)
endif()

# Common configuration file
#
set(SAIL_CODECS_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/sail/codecs")
if (WIN32)
    string(REPLACE "/" "\\\\" SAIL_CODECS_PATH "${SAIL_CODECS_PATH}")
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

add_subdirectory(src/libsail-common)
add_subdirectory(src/libsail)
add_subdirectory(src/sail-codecs)
add_subdirectory(src/bindings/c++)

if (SAIL_BUILD_EXAMPLES)
    add_subdirectory(examples/c/sail-convert)
    add_subdirectory(examples/c/sail-probe)

    find_package(SDL2)
    set(SAIL_SDL_EXAMPLE OFF)

    if (SDL2_FOUND)
        set(SAIL_SDL_EXAMPLE ON)
        add_subdirectory(examples/c/sail-sdl-viewer)
    endif()
endif()

if (SAIL_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Installation
#
install(FILES "${PROJECT_BINARY_DIR}/include/config.h" DESTINATION include/sail/sail-common)

if (WIN32)
    if (EXISTS "${PROJECT_SOURCE_DIR}/extra/B/bin")
        install(DIRECTORY "${PROJECT_SOURCE_DIR}/extra/B/bin/" DESTINATION lib/sail/codecs/lib)
    endif()
else()
    if (EXISTS "${PROJECT_SOURCE_DIR}/extra/B/lib")
        install(DIRECTORY "${PROJECT_SOURCE_DIR}/extra/B/lib/" DESTINATION lib/sail/codecs/lib)
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
message("* CMake C flags:               ${CMAKE_C_FLAGS}")
message("* CMake CXX flags:             ${CMAKE_CXX_FLAGS}")
message("* CMake shared link flags:     ${CMAKE_SHARED_LINKER_FLAGS}")
message("* CMake static link flags:     ${CMAKE_STATIC_LINKER_FLAGS}")
message("* CMake module link flags:     ${CMAKE_MODULE_LINKER_FLAGS}")
message("*")
message("* SAIL version:                ${PROJECT_VERSION}")
message("* Developer mode:              ${SAIL_DEV}")
message("* VCPKG mode:                  ${SAIL_VCPKG}")
message("* SDL example:                 ${SAIL_SDL_EXAMPLE}")
message("*")
message("* Install prefix:              ${CMAKE_INSTALL_PREFIX}")
message("* LIBDIR:                      ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
message("* INCLUDEDIR:                  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
message("* DATADIR:                     ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}")
message("*")
message("* Colored output:              ${SAIL_COLORED_OUTPUT}${SAIL_COLORED_OUTPUT_CLARIFY}")
message("* Enabled codecs:              ${ENABLED_CODECS}")
message("* Disabled codecs:             ${DISABLED_CODECS}")
message("* Default read output format:  ${SAIL_DEFAULT_READ_OUTPUT_PIXEL_FORMAT}")
message("*")
message("***************************************")
message("")
