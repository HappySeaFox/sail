cmake_minimum_required(VERSION 3.6.0)

# Enable strict C++11
#
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

project(qt-sail-demo-novice)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(PkgConfig REQUIRED)

# Manifest to support Windows 10 is embedded into our RC file
#
set(RC_FILE "")
if (WIN32)
    set(RC_FILE "qtsail.rc")
    set(CMAKE_EXE_LINKER_FLAGS "/MANIFEST:NO")
endif()

add_executable(qt-sail-demo-novice
                main.cpp
                qtsail.cpp
                qtsail.ui
                "${RC_FILE}")

# Enable ASAN
#
if (SAIL_ASAN)
    target_compile_options(qt-sail-demo-novice "${SAIL_ASAN}")
    target_link_libraries(qt-sail-demo-novice "${SAIL_ASAN}")
endif()

# Add SAIL dependency
#
pkg_check_modules(SAIL IMPORTED_TARGET REQUIRED libsail)
target_compile_options(qt-sail-demo-novice     PRIVATE "${SAIL_CFLAGS}")
target_include_directories(qt-sail-demo-novice PRIVATE "${SAIL_INCLUDE_DIRS}")
target_link_libraries(qt-sail-demo-novice      PkgConfig::SAIL)

# Add Qt5 dependency
#
find_package(Qt5 COMPONENTS Widgets REQUIRED)
target_link_libraries(qt-sail-demo-novice Qt5::Widgets)
