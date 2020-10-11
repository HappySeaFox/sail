# Options
#
option(SAIL_COLORED_OUTPUT "Enable colored console output on supported platforms" ON)

# Export options to the parent cmake file to print statistics
#
set(SAIL_COLORED_OUTPUT ${SAIL_COLORED_OUTPUT} PARENT_SCOPE)

add_library(sail-common
                src/iccp.c
                src/image.c
                src/io_common.c
                src/log.c
                src/meta_data_node.c
                src/palette.c
                src/pixel_formats_mapping_node.c
                src/read_features.c
                src/read_options.c
                src/resolution.c
                src/source_image.c
                src/utils.c
                src/write_features.c
                src/write_options.c)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS "src/common.h"
                   "src/error.h"
                   "src/export.h"
                   "src/iccp.h"
                   "src/image.h"
                   "src/io_common.h"
                   "src/log.h"
                   "src/meta_data_node.h"
                   "src/palette.h"
                   "src/pixel_formats_mapping_node.h"
                   "src/read_features.h"
                   "src/read_options.h"
                   "src/resolution.h"
                   "src/source_image.h"
                   "src/sail-common.h"
                   "src/utils.h"
                   "src/write_features.h"
                   "src/write_options.h")

set_target_properties(sail-common PROPERTIES
                                  VERSION "0.7.2"
                                  SOVERSION 0
                                  PUBLIC_HEADER "${PUBLIC_HEADERS}")

sail_enable_pch(TARGET sail-common HEADER src/sail-common.h)

# Definitions, includes, link
#
if (SAIL_COLORED_OUTPUT)
    target_compile_definitions(sail-common PRIVATE SAIL_COLORED_OUTPUT=1)
endif()
target_include_directories(sail-common PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

# pkg-config integration
#
get_target_property(VERSION sail-common VERSION)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/libsail-common.pc.in"
                "${CMAKE_CURRENT_BINARY_DIR}/libsail-common.pc" @ONLY)

# Installation
#
install(TARGETS sail-common
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-common")

# Install development packages
#
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libsail-common.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
