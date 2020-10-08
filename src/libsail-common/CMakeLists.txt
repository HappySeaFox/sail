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
                src/meta_entry_node.c
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
set(PUBLIC_HEADERS "")
list(APPEND PUBLIC_HEADERS "src/common.h")
list(APPEND PUBLIC_HEADERS "src/error.h")
list(APPEND PUBLIC_HEADERS "src/export.h")
list(APPEND PUBLIC_HEADERS "src/iccp.h")
list(APPEND PUBLIC_HEADERS "src/image.h")
list(APPEND PUBLIC_HEADERS "src/io_common.h")
list(APPEND PUBLIC_HEADERS "src/log.h")
list(APPEND PUBLIC_HEADERS "src/meta_entry_node.h")
list(APPEND PUBLIC_HEADERS "src/palette.h")
list(APPEND PUBLIC_HEADERS "src/pixel_formats_mapping_node.h")
list(APPEND PUBLIC_HEADERS "src/read_features.h")
list(APPEND PUBLIC_HEADERS "src/read_options.h")
list(APPEND PUBLIC_HEADERS "src/resolution.h")
list(APPEND PUBLIC_HEADERS "src/source_image.h")
list(APPEND PUBLIC_HEADERS "src/sail-common.h")
list(APPEND PUBLIC_HEADERS "src/utils.h")
list(APPEND PUBLIC_HEADERS "src/write_features.h")
list(APPEND PUBLIC_HEADERS "src/write_options.h")

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
