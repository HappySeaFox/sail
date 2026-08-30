cmake_minimum_required(VERSION 3.16)

project(thumbnailer-with-c++-api CXX)

# Enable strict C++17
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to enable ASAN on supported platforms
#
option(SAIL_ASAN "Enable ASAN with supported compilers." ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 is preferred, Qt5 is still supported
#
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

# Add SAIL dependency. This example demonstrates linking with CMake packages
#
find_package(SailC++ REQUIRED)

# Manifest to support Windows 10 is embedded into our RC file
#
set(RC_FILE "")
if (WIN32)
    set(RC_FILE "thumbnailer.rc")
endif()

add_executable(thumbnailer-with-c++-api
                main.cpp
                thumbnailer.cpp
                thumbnail_loader.cpp
                thumbnailer.ui
                "${RC_FILE}")

if (MSVC)
    target_link_options(thumbnailer-with-c++-api PRIVATE "/MANIFEST:NO")
endif()

# Enable ASAN
#
if (SAIL_ASAN)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(thumbnailer-with-c++-api PRIVATE "-fsanitize=address")
        target_link_options(thumbnailer-with-c++-api PRIVATE "-fsanitize=address")
    else()
        message(STATUS "ASAN is not supported with this compiler.")
    endif()
endif()

target_link_libraries(thumbnailer-with-c++-api PRIVATE Qt${QT_VERSION_MAJOR}::Widgets SAIL::sail-c++)
