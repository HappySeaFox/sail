cmake_minimum_required(VERSION 3.12)

project(SAIL VERSION 0.9.8
             DESCRIPTION "Squirrel Abstract Image Library"
             LANGUAGES C CXX)

include(GNUInstallDirs)
include(CheckIncludeFiles)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CMakeDependentOption)
include(CMakePushCheckState)
include(CTest)

# Our own cmake scripts
#
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
include(sail_check_alignas)
include(sail_check_builtin_bswap)
include(sail_check_c11_thread_local)
include(sail_check_include)
include(sail_check_init_once_execute_once)
include(sail_check_openmp)
include(sail_codec)
include(sail_enable_asan)
include(sail_enable_pch)
include(sail_enable_posix_source)
include(sail_enable_xopen_source)
include(sail_install_cmake_config)
include(sail_install_pdb)
include(sail_test)

include(JoinPaths)

# https://github.com/jtojnar/cmake-snips#concatenating-paths-when-building-pkg-config-files
#
join_paths(SAIL_LIBDIR_FOR_PKG_CONFIG     "\${exec_prefix}" "${CMAKE_INSTALL_LIBDIR}")
join_paths(SAIL_INCLUDEDIR_FOR_PKG_CONFIG "\${prefix}"      "${CMAKE_INSTALL_INCLUDEDIR}")

# Check features
#
sail_check_alignas()
sail_check_builtin_bswap()
sail_check_c11_thread_local()

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
option(SAIL_BUILD_APPS "Build applications." ON)
option(SAIL_BUILD_BINDINGS "Build the C++ and other bindings." ON)
option(SAIL_BUILD_EXAMPLES "Build examples." ON)
option(SAIL_DEV "Enable developer mode. Be more strict when compiling source code, for example." OFF)
option(SAIL_ENABLE_OPENMP "Enable OpenMP support if it's available in the compiler." ON)
set(SAIL_ENABLE_CODECS "" CACHE STRING "Forcefully enable the codecs specified in this ';'-separated list. \
If an enabled codec fails to find its dependencies, the configuration process fails. \
One can also specify not just individual codecs but codec groups by their priority like that: highest-priority;xbm. \
Other codecs may or may not be enabled depending on found dependencies. \
When SAIL_ENABLE_CODECS is enabled, SAIL_ONLY_CODECS gets ignored.")
set(SAIL_DISABLE_CODECS "" CACHE STRING "Disable the codecs specified in this ';'-separated list. \
One can also specify not just individual codecs but codec groups by their priority like that: highest-priority;xbm.")
option(SAIL_INSTALL_PDB "Install PDB files along with libraries." ON)
set(SAIL_ONLY_CODECS "" CACHE STRING "Forcefully enable only the codecs specified in this ';'-separated list and disable the rest. \
If an enabled codec fails to find its dependencies, the configuration process fails. \
One can also specify not just individual codecs but codec groups by their priority like that: highest-priority;xbm.")
set(SAIL_OPENMP_SCHEDULE "dynamic" CACHE STRING "OpenMP scheduling algorithm.")
option(BUILD_SHARED_LIBS "Build shared libs. When disabled, sets SAIL_COMBINE_CODECS to ON automatically." ON)
cmake_dependent_option(SAIL_COMBINE_CODECS "Combine all codecs into a single library. When disabled, all codecs are implemented as \
dynamically loaded plugins." OFF "BUILD_SHARED_LIBS" ON)
option(SAIL_THIRD_PARTY_CODECS_PATH "Enable loading third-party codecs from the ';'-separated paths specified in \
the SAIL_THIRD_PARTY_CODECS_PATH environment variable." ON)
option(SAIL_THREAD_SAFE "Enable working in multi-threaded environments by locking the internal context with a mutex." ON)
if (WIN32)
    option(SAIL_WINDOWS_UTF8_PATHS "Convert file paths to UTF-8 on Windows." ON)
endif()

if (SAIL_ENABLE_OPENMP)
    sail_check_openmp()
else()
    set(SAIL_HAVE_OPENMP_DISPLAY "OFF (forced)" CACHE INTERNAL "")
endif()

# When we compile for VCPKG, VCPKG_TARGET_TRIPLET is defined
#
if (VCPKG_TARGET_TRIPLET)
    set(SAIL_VCPKG ON)
else()
    set(SAIL_VCPKG OFF)
endif()

# Number of bytes to read from a file or memory to detect the image
# format by its MIME type.
#
set(SAIL_MAGIC_BUFFER_SIZE 16)

# Our bundled libs
#
if (WIN32 AND NOT SAIL_VCPKG)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PROJECT_SOURCE_DIR}/extra/B)

    # For check_c_source_runs() we need to update PATH
    # so checks are able to find bundled libs
    #
    set(EXTRA_LIBS "${PROJECT_SOURCE_DIR}/extra/B/bin")
    string(REPLACE "/" "\\" EXTRA_LIBS "${EXTRA_LIBS}")
    set(ENV{PATH} "$ENV{PATH};${EXTRA_LIBS}")
endif()

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

# Build position-independent targets
#
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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

if (MINGW)
    set(SAIL_MINGW ON)
endif()

if (CYGWIN)
    set(SAIL_CYGWIN ON)
endif()

if (APPLE)
    set(SAIL_APPLE ON)
endif()

if (UNIX)
    set(SAIL_UNIX ON)
endif()

# Codecs & icons paths
#
set(SAIL_CODECS_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/sail/codecs")
if (WIN32)
    string(REPLACE "/" "\\\\" SAIL_CODECS_PATH "${SAIL_CODECS_PATH}")
endif()

# Global include directory with generated configs
#
include_directories("${PROJECT_BINARY_DIR}/include")

# Configure subdirs
#
add_subdirectory(src/sail-common)
add_subdirectory(src/sail-codecs)
if (SAIL_COMBINE_CODECS)
    add_subdirectory(src/sail-codecs-archive)
endif()
add_subdirectory(src/sail)
add_subdirectory(src/sail-manip)
if (SAIL_BUILD_BINDINGS)
  add_subdirectory(src/bindings/sail-c++)
endif()

if (SAIL_BUILD_APPS)
    add_subdirectory(examples/c/sail)
endif()

if (SAIL_BUILD_EXAMPLES)
    find_package(SDL2)
    set(SAIL_SDL_EXAMPLE OFF)

    if (SDL2_FOUND)
        set(SAIL_SDL_EXAMPLE ON)
        add_subdirectory(examples/c/sail-sdl-viewer)
    endif()
endif()

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Error check: This particular build of SAIL cannot load any image
#
if (NOT ENABLED_CODECS AND NOT SAIL_THIRD_PARTY_CODECS_PATH)
    message(FATAL_ERROR "No codecs are enabled and SAIL_THIRD_PARTY_CODECS_PATH is disabled.\nThis particular build of SAIL cannot load any image.")
endif()

# Install our bundled libs if they exist and when VCPKG is not used
#
if (NOT SAIL_VCPKG)
    # When SAIL_COMBINE_CODECS is ON, sail-codecs.dll directly depends on extra libs.
    # Copy them into the bin directory. Otherwise, client applications will fail to run.
    #
    # When SAIL_COMBINE_CODECS is OFF, SAIL loads codecs in runtime. In this case we can put
    # extra libs in a separate directory and update the DLL search path.
    #
    if (SAIL_COMBINE_CODECS)
        set(SAIL_EXTRA_LIBS_INSTALL_PATH "bin")
    else()
        set(SAIL_EXTRA_LIBS_INSTALL_PATH "lib/sail/codecs/lib")
    endif()

    if (WIN32)
        if (EXISTS "${PROJECT_SOURCE_DIR}/extra/B/bin")
            install(DIRECTORY "${PROJECT_SOURCE_DIR}/extra/B/bin/" DESTINATION ${SAIL_EXTRA_LIBS_INSTALL_PATH})
        endif()
        if (EXISTS "${PROJECT_SOURCE_DIR}/extra/B/share/sail/licenses")
            install(DIRECTORY "${PROJECT_SOURCE_DIR}/extra/B/share/sail/licenses/" DESTINATION share/sail/licenses)
        endif()
    else()
        if (EXISTS "${PROJECT_SOURCE_DIR}/extra/B/lib")
            install(DIRECTORY "${PROJECT_SOURCE_DIR}/extra/B/lib/" DESTINATION ${SAIL_EXTRA_LIBS_INSTALL_PATH})
        endif()
    endif()
endif()

if (ENABLED_CODECS)
    string(TOUPPER "${ENABLED_CODECS}" ENABLED_CODECS)

    foreach (codec IN LISTS ENABLED_CODECS)
        set(SAIL_HAVE_CODEC_DEFINES "${SAIL_HAVE_CODEC_DEFINES}#define SAIL_HAVE_BUILTIN_${codec}\n")
    endforeach()

    string(REPLACE ";" " " ENABLED_CODECS "${ENABLED_CODECS}")
endif()

if (DISABLED_CODECS)
    string(TOUPPER "${DISABLED_CODECS}" DISABLED_CODECS)
    string(REPLACE ";" " " DISABLED_CODECS "${DISABLED_CODECS}")
endif()

# Common configuration file
#
configure_file("${PROJECT_SOURCE_DIR}/src/config.h.in" "${PROJECT_BINARY_DIR}/include/sail-common/config.h" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/include/sail-common/config.h" DESTINATION include/sail/sail-common)

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
message("* CMake version:                ${CMAKE_VERSION}")
message("* CMake C flags:                ${CMAKE_C_FLAGS}")
message("* CMake CXX flags:              ${CMAKE_CXX_FLAGS}")
message("* CMake shared link flags:      ${CMAKE_SHARED_LINKER_FLAGS}")
message("* CMake static link flags:      ${CMAKE_STATIC_LINKER_FLAGS}")
message("* CMake module link flags:      ${CMAKE_MODULE_LINKER_FLAGS}")
message("*")
message("* SAIL version:                 ${PROJECT_VERSION}")
message("* Developer mode:               ${SAIL_DEV}")
message("* VCPKG mode:                   ${SAIL_VCPKG}")
message("* Shared build:                 ${BUILD_SHARED_LIBS}")
message("*   Combine codecs [*]:         ${SAIL_COMBINE_CODECS}")
message("* Thread-safe:                  ${SAIL_THREAD_SAFE}")
message("* SAIL_THIRD_PARTY_CODECS_PATH: ${SAIL_THIRD_PARTY_CODECS_PATH}")
message("* Colored output:               ${SAIL_COLORED_OUTPUT}${SAIL_COLORED_OUTPUT_CLARIFY}")
message("* Build apps:                   ${SAIL_BUILD_APPS}")
message("* Build examples:               ${SAIL_BUILD_EXAMPLES}")
message("* Build SDL example:            ${SAIL_SDL_EXAMPLE}")
message("* Build bindings:               ${SAIL_BUILD_BINDINGS}")
message("* Build tests:                  ${BUILD_TESTING}")
message("* Install PDB files:            ${SAIL_INSTALL_PDB}")
message("*")
message("* SAIL_HAVE_BUILTIN_BSWAP16:    ${SAIL_HAVE_BUILTIN_BSWAP16_DISPLAY}")
message("* SAIL_HAVE_BUILTIN_BSWAP32:    ${SAIL_HAVE_BUILTIN_BSWAP32_DISPLAY}")
message("* SAIL_HAVE_BUILTIN_BSWAP64:    ${SAIL_HAVE_BUILTIN_BSWAP64_DISPLAY}")
message("* SAIL_HAVE_OPENMP:             ${SAIL_HAVE_OPENMP_DISPLAY}")
message("* SAIL_OPENMP_SCHEDULE:         ${SAIL_OPENMP_SCHEDULE}")
message("* SAIL_OPENMP_FLAGS:            ${SAIL_OPENMP_FLAGS}")
message("* SAIL_OPENMP_INCLUDE_DIRS:     ${SAIL_OPENMP_INCLUDE_DIRS}")
message("* SAIL_OPENMP_LIBS:             ${SAIL_OPENMP_LIBS}")
if (WIN32)
    message("* SAIL_WINDOWS_UTF8_PATHS:      ${SAIL_WINDOWS_UTF8_PATHS}")
endif()
message("*")
message("* [*] - these options depend on other options, their values may be altered by CMake.")
message("*       For example, if you configure with -DBUILD_SHARED_LIBS=OFF -DSAIL_COMBINE_CODECS=OFF,")
message("*       the final value of SAIL_COMBINE_CODECS will be ON.")
message("*")
message("* Install prefix:               ${CMAKE_INSTALL_PREFIX}")
message("* LIBDIR:                       ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
message("* INCLUDEDIR:                   ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
message("* DATADIR:                      ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}")
message("*")
message("* Enabled codecs:               ${ENABLED_CODECS}")
message("* Disabled codecs:              ${DISABLED_CODECS}")
message("*")
message("***************************************")
message("")
