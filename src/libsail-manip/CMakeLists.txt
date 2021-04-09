add_library(sail-manip
                cmyk.c
                cmyk.h
                convert_to_32.c
                convert_to_32.h
                convert_to_64.c
                convert_to_64.h
                pixel_utils.c
                pixel_utils.h
                sail-manip.h
                ycbcr.c
                ycbcr.h)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS "convert_to_32.h"
                   "convert_to_64.h"
                   "sail-manip.h")

set_target_properties(sail-manip PROPERTIES
                                 VERSION "0.3.0"
                                 SOVERSION 0
                                 PUBLIC_HEADER "${PUBLIC_HEADERS}")

sail_enable_pch(TARGET sail-manip HEADER sail-manip.h)

# Definitions, includes, link
#
target_include_directories(sail-manip PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(sail-manip PUBLIC sail-common)

# pkg-config integration
#
get_target_property(VERSION sail-manip VERSION)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/libsail-manip.pc.in"
                "${CMAKE_CURRENT_BINARY_DIR}/libsail-manip.pc" @ONLY)

# Installation
#
install(TARGETS sail-manip
        EXPORT SailManipTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-manip")

# Install development packages
#
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libsail-manip.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

sail_install_cmake_config(TARGET "SailManip" VERSION "${VERSION}")
