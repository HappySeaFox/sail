# Options
#
option(SAIL_COLORED_OUTPUT "Enable colored console output on supported platforms" ON)

# Export options to the parent cmake file to print statistics
#
set(SAIL_COLORED_OUTPUT ${SAIL_COLORED_OUTPUT} PARENT_SCOPE)

add_library(sail-common
                common.h
                common_serialize.c
                common_serialize.h
                compiler_specifics.h
                error.h
                export.h
                iccp.c
                iccp.h
                image.c
                image.h
                io_common.c
                io_common.h
                log.c
                log.h
                memory.c
                memory.h
                meta_data_node.c
                meta_data_node.h
                palette.c
                palette.h
                pixel.c
                pixel.h
                read_features.c
                read_features.h
                read_options.c
                read_options.h
                resolution.c
                resolution.h
                sail-common.h
                source_image.c
                source_image.h
                utils.c
                utils.h
                vector.c
                vector.h
                write_features.c
                write_features.h
                write_options.c
                write_options.h)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS "common.h"
                   "common_serialize.h"
                   "compiler_specifics.h"
                   "error.h"
                   "export.h"
                   "iccp.h"
                   "image.h"
                   "io_common.h"
                   "log.h"
                   "memory.h"
                   "meta_data_node.h"
                   "palette.h"
                   "pixel.h"
                   "read_features.h"
                   "read_options.h"
                   "resolution.h"
                   "sail-common.h"
                   "source_image.h"
                   "utils.h"
                   "vector.h"
                   "write_features.h"
                   "write_options.h")

set_target_properties(sail-common PROPERTIES
                                  VERSION "0.9.2"
                                  SOVERSION 0
                                  PUBLIC_HEADER "${PUBLIC_HEADERS}")

# fileno
sail_enable_posix_source(TARGET sail-common VERSION 200112L)

sail_enable_pch(TARGET sail-common HEADER sail-common.h)

# Definitions, includes, link
#
if (SAIL_COLORED_OUTPUT)
    target_compile_definitions(sail-common PRIVATE SAIL_COLORED_OUTPUT=1)
endif()
target_include_directories(sail-common
                            PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                   $<INSTALL_INTERFACE:include/sail>)

# pkg-config integration
#
get_target_property(VERSION sail-common VERSION)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/libsail-common.pc.in"
                "${CMAKE_CURRENT_BINARY_DIR}/libsail-common.pc" @ONLY)

# Installation
#
install(TARGETS sail-common
        EXPORT SailCommonTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-common")

# Install development packages
#
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libsail-common.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

sail_install_cmake_config(TARGET "SailCommon" FOLDER "sailcommon" VERSION "${VERSION}")
