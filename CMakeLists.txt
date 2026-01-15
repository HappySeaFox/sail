cmake_minimum_required(VERSION 3.18)

project(SAIL VERSION 1.0.0
             DESCRIPTION "Squirrel Abstract Imaging Library"
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
include(sail_enable_tsan)
include(sail_enable_no_undefined)
include(sail_enable_pch)
include(sail_enable_pic)
include(sail_enable_posix_c_source)
include(sail_enable_xopen_source)
include(sail_enable_warnings)
include(sail_install_cmake_config)
include(sail_test)
include(sail_windows_enable_exception_handling)
include(sail_windows_install_pdb)
include(sail_windows_set_crt)

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
sail_check_include(fcntl.h)
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
option(SAIL_BUILD_APPS "Build client applications." ON)
option(SAIL_BUILD_BINDINGS "Build C++ and other bindings." ON)
option(SAIL_BUILD_EXAMPLES "Build examples." ON)
option(SAIL_ASAN "Enable AddressSanitizer." OFF)
option(SAIL_TSAN "Enable ThreadSanitizer." OFF)
option(SAIL_ENABLE_OPENMP "Enable OpenMP support if available in the compiler. See also SAIL_OPENMP_SCHEDULE." ON)
set(SAIL_ENABLE_CODECS "" CACHE STRING "Force-enable the codecs specified in this ';'-separated list. \
Configuration fails if an enabled codec cannot find its dependencies. \
Supports individual codecs and codec groups by priority (e.g., highest-priority;xbm). \
Other codecs may be enabled or disabled based on available dependencies. \
When set, SAIL_ONLY_CODECS is ignored.")
set(SAIL_DISABLE_CODECS "" CACHE STRING "Disable the codecs specified in this ';'-separated list. \
Supports individual codecs and codec groups by priority (e.g., highest-priority;xbm).")
set(SAIL_ONLY_CODECS "" CACHE STRING "Force-enable only the codecs specified in this ';'-separated list and disable all others. \
Configuration fails if an enabled codec cannot find its dependencies. \
Supports individual codecs and codec groups by priority (e.g., highest-priority;xbm).")
set(SAIL_OPENMP_SCHEDULE "dynamic" CACHE STRING "OpenMP scheduling algorithm.")
option(BUILD_SHARED_LIBS "Build shared libraries. When disabled, automatically sets SAIL_COMBINE_CODECS to ON." ON)
cmake_dependent_option(SAIL_COMBINE_CODECS "Combine all codecs into a single library. When disabled, all codecs are implemented as \
dynamically loaded plugins." OFF "BUILD_SHARED_LIBS" ON)
option(SAIL_THIRD_PARTY_CODECS_PATH "Enable loading custom codecs from ';'-separated paths specified in \
the SAIL_THIRD_PARTY_CODECS_PATH environment variable." ON)
option(SAIL_THREAD_SAFE "Enable thread-safe operations by locking the internal context with a mutex." ON)
if (WIN32)
    option(SAIL_WINDOWS_UTF8_PATHS "Convert file paths to UTF-8 on Windows." ON)
    if (MSVC)
        option(SAIL_WINDOWS_INSTALL_PDB "Install PDB debug files along with libraries." ON)
        option(SAIL_WINDOWS_STATIC_CRT "Use static CRT (/MT) instead of dynamic CRT (/MD) for static builds." ON)
    endif()
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

# Create common flags interface library and enable warnings/ASAN/TSAN/PIC/undefined flags
#
add_library(sail-common-flags INTERFACE)
sail_enable_warnings(INTERFACE_LIB sail-common-flags)
sail_enable_asan(INTERFACE_LIB sail-common-flags)
sail_enable_tsan(INTERFACE_LIB sail-common-flags)
sail_enable_pic(INTERFACE_LIB sail-common-flags)
sail_enable_no_undefined(INTERFACE_LIB sail-common-flags)
sail_windows_enable_exception_handling(INTERFACE_LIB sail-common-flags)

# Enable C11
#
target_compile_features(sail-common-flags INTERFACE c_std_11)

# Enable C++11
#
target_compile_features(sail-common-flags INTERFACE cxx_std_11)

# Internal flag used to include SAIL headers locally with "header.h" or <sail/header.h> otherwise
#
target_compile_definitions(sail-common-flags INTERFACE SAIL_BUILD)

# Windows CRT selection for static builds
#
if (WIN32 AND MSVC)
    sail_windows_set_crt(INTERFACE_LIB sail-common-flags STATIC_CRT ${SAIL_WINDOWS_STATIC_CRT})

    if (BUILD_SHARED_LIBS)
        set(SAIL_WINDOWS_STATIC_CRT_DISPLAY "Dynamic (/MD) - shared build")
    else()
        if (SAIL_WINDOWS_STATIC_CRT)
            set(SAIL_WINDOWS_STATIC_CRT_DISPLAY "Static (/MT)")
        else()
            set(SAIL_WINDOWS_STATIC_CRT_DISPLAY "Dynamic (/MD)")
        endif()
    endif()
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
    add_subdirectory(man)
endif()

if (SAIL_BUILD_EXAMPLES)
    find_package(SDL2 QUIET)
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

# Install VCPKG libs on Windows only. On Unix platforms vcpkg uses rpath
#
if (SAIL_VCPKG AND WIN32)
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

    if (NOT CMAKE_BUILD_TYPE)
        message(FATAL_ERROR "CMAKE_BUILD_TYPE is required for VCPKG build")
    endif()

    if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        set(SAIL_VCPKG_BIN_PATH "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
    else()
        set(SAIL_VCPKG_BIN_PATH "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin")
    endif()
    get_filename_component(SAIL_VCPKG_BIN_PATH "${SAIL_VCPKG_BIN_PATH}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    message(STATUS "Install VCPKG libs '${SAIL_VCPKG_BIN_PATH}/' to '${SAIL_EXTRA_LIBS_INSTALL_PATH}'")
    install(DIRECTORY "${SAIL_VCPKG_BIN_PATH}/" DESTINATION ${SAIL_EXTRA_LIBS_INSTALL_PATH} OPTIONAL)
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

get_target_property(SAIL_COMMON_COMPILE_OPTIONS     sail-common-flags INTERFACE_COMPILE_OPTIONS)
get_target_property(SAIL_COMMON_COMPILE_DEFINITIONS sail-common-flags INTERFACE_COMPILE_DEFINITIONS)
get_target_property(SAIL_COMMON_LINK_OPTIONS        sail-common-flags INTERFACE_LINK_OPTIONS)

if (NOT SAIL_COMMON_COMPILE_OPTIONS)
    set(SAIL_COMMON_COMPILE_OPTIONS "")
endif()
if (NOT SAIL_COMMON_COMPILE_DEFINITIONS)
    set(SAIL_COMMON_COMPILE_DEFINITIONS "")
endif()
if (NOT SAIL_COMMON_LINK_OPTIONS)
    set(SAIL_COMMON_LINK_OPTIONS "")
endif()

message("")
message("***************************************")
message("*")
message("* Configuration statistics: ")
message("*")
message("* CMake version:                ${CMAKE_VERSION}")
message("* Common compile options:       ${SAIL_COMMON_COMPILE_OPTIONS}")
message("* Common compile definitions:   ${SAIL_COMMON_COMPILE_DEFINITIONS}")
message("* Common link options:          ${SAIL_COMMON_LINK_OPTIONS}")
message("*")
message("* SAIL version:                 ${PROJECT_VERSION}")
message("* AddressSanitizer:             ${SAIL_ASAN}")
message("* ThreadSanitizer:              ${SAIL_TSAN}")
message("* VCPKG mode:                   ${SAIL_VCPKG}")
message("* Shared build:                 ${BUILD_SHARED_LIBS}")
message("*   Combine codecs [*]:         ${SAIL_COMBINE_CODECS}")
if (WIN32 AND MSVC)
    message("* CRT:                          ${SAIL_WINDOWS_STATIC_CRT_DISPLAY}")
endif()
message("* Thread-safe:                  ${SAIL_THREAD_SAFE}")
message("* SAIL_THIRD_PARTY_CODECS_PATH: ${SAIL_THIRD_PARTY_CODECS_PATH}")
message("* Colored output:               ${SAIL_COLORED_OUTPUT}${SAIL_COLORED_OUTPUT_CLARIFY}")
message("* Build apps:                   ${SAIL_BUILD_APPS}")
message("* Build examples:               ${SAIL_BUILD_EXAMPLES}")
message("* Build SDL example:            ${SAIL_SDL_EXAMPLE}")
message("* Build bindings:               ${SAIL_BUILD_BINDINGS}")
message("* Build tests:                  ${BUILD_TESTING}")
if (MSVC)
    message("* Install PDB files:            ${SAIL_WINDOWS_INSTALL_PDB}")
endif()
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
