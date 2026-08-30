cmake_minimum_required(VERSION 3.16)

project(advanced-with-c++-api CXX)

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

# Add the image viewer widget shared by all the Qt examples. It needs SAIL headers as well
#
set(SAIL_QT_EXAMPLE_SAIL_TARGET SAIL::sail-c++)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../common" common)

# Manifest to support Windows 10 is embedded into our RC file
#
set(RC_FILE "")
if (WIN32)
    set(RC_FILE "qtsail.rc")
endif()

add_executable(advanced-with-c++-api
                main.cpp
                qtsail.cpp
                "${RC_FILE}")

if (MSVC)
    target_link_options(advanced-with-c++-api PRIVATE "/MANIFEST:NO")
endif()

# Enable ASAN
#
if (SAIL_ASAN)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(advanced-with-c++-api PRIVATE "-fsanitize=address")
        target_link_options(advanced-with-c++-api PRIVATE "-fsanitize=address")
    else()
        message(STATUS "ASAN is not supported with this compiler.")
    endif()
endif()

target_link_libraries(advanced-with-c++-api PRIVATE sail-qt-example-viewer SAIL::sail-c++)
