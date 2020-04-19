cmake_minimum_required(VERSION 3.6.0)

# Enable strict C++11
#
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to ensable ASAN on supported platforms
#
option(SAIL_ASAN "Enable ASAN on supported platforms." ON)

project(deep-diver-with-c-api)

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

add_executable(deep-diver-with-c-api
                main.cpp
                qtsail.cpp
                qtsail.ui
                readoptions.cpp
                readoptions.ui
                writeoptions.cpp
                writeoptions.ui
                "${RC_FILE}")

# Enable ASAN
#
if (SAIL_ASAN)
    if (CMAKE_COMPILER_IS_GNUCC)
        target_compile_options(deep-diver-with-c-api PRIVATE "-fsanitize=address,leak")
        target_link_libraries(deep-diver-with-c-api "-fsanitize=address,leak")
    else()
        message("ASAN is not supported on this platform.")
    endif()
endif()

# Add SAIL dependency
#
pkg_check_modules(SAIL IMPORTED_TARGET REQUIRED libsail)
target_compile_options(deep-diver-with-c-api     PRIVATE "${SAIL_CFLAGS}")
target_include_directories(deep-diver-with-c-api PRIVATE "${SAIL_INCLUDE_DIRS}")
target_link_libraries(deep-diver-with-c-api      PkgConfig::SAIL)

# Add Qt5 dependency
#
find_package(Qt5 COMPONENTS Widgets REQUIRED)
target_link_libraries(deep-diver-with-c-api Qt5::Widgets)
