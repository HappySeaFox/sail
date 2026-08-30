# The image viewer widget shared by all the Qt examples. It implements everything but
# the SAIL calls so that every example is left with SAIL code only.
#
# Include this directory from an example after setting SAIL_QT_EXAMPLE_SAIL_TARGET to the SAIL
# target the widget must be compiled against. The widget needs SAIL headers only, so every
# example passes the SAIL flavor it links itself.
#
if (NOT DEFINED SAIL_QT_EXAMPLE_SAIL_TARGET)
    message(FATAL_ERROR "Set SAIL_QT_EXAMPLE_SAIL_TARGET to the SAIL target to compile the viewer against.")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 is preferred, Qt5 is still supported
#
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

add_library(sail-qt-example-viewer STATIC
                image_viewer.cpp
                image_viewer.ui)

# image_viewer.h and qimage_sail_pixel_formats.h are included by the examples
#
target_include_directories(sail-qt-example-viewer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# The viewer exposes Qt types and sail_status_t in its interface
#
target_link_libraries(sail-qt-example-viewer PUBLIC Qt${QT_VERSION_MAJOR}::Widgets ${SAIL_QT_EXAMPLE_SAIL_TARGET})

# Enable ASAN
#
if (SAIL_ASAN)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(sail-qt-example-viewer PRIVATE "-fsanitize=address")
        target_link_options(sail-qt-example-viewer PRIVATE "-fsanitize=address")
    endif()
endif()
