add_library(sail
                src/ini.c
                src/io_file.c
                src/io_mem.c
                src/io_noop.c
                src/codec.c
                src/codec_info.c
                src/codec_info_node.c
                src/codec_info_private.c
                src/context.c
                src/sail_advanced.c
                src/sail_deep_diver.c
                src/sail_junior.c
                src/sail_private.c
                src/sail_technical_diver.c
                src/sail_technical_diver_private.c)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS "src/codec_info.h"
                   "src/codec_info_node.h"
                   "src/context.h"
                   "src/sail.h"
                   "src/sail_advanced.h"
                   "src/sail_deep_diver.h"
                   "src/sail_junior.h"
                   "src/sail_technical_diver.h"
                   "src/string_node.h")

set_target_properties(sail PROPERTIES
                           VERSION "0.7.0"
                           SOVERSION 0
                           PUBLIC_HEADER "${PUBLIC_HEADERS}")

sail_enable_pch(TARGET sail HEADER src/sail.h)

# Definitions, includes, link
#
target_include_directories(sail PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
target_compile_definitions(sail PRIVATE INI_STOP_ON_FIRST_ERROR=1)
target_link_libraries(sail PUBLIC sail-common)

if (UNIX)
    target_link_libraries(sail PRIVATE dl)
endif()

# pkg-config integration
#
get_target_property(VERSION sail VERSION)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/libsail.pc.in"
                "${CMAKE_CURRENT_BINARY_DIR}/libsail.pc" @ONLY)

# Installation
#
install(TARGETS sail
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail/sail")

# Install development packages
#
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libsail.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
