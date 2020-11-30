add_library(sail-c++
                context-c++.cpp
                iccp-c++.cpp
                image-c++.cpp
                image_reader-c++.cpp
                image_writer-c++.cpp
                io_common-c++.cpp
                log-c++.cpp
                meta_data-c++.cpp
                palette-c++.cpp
                codec_info-c++.cpp
                read_features-c++.cpp
                read_options-c++.cpp
                resolution-c++.cpp
                source_image-c++.cpp
                write_features-c++.cpp
                write_options-c++.cpp
                utils-c++.cpp)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS "at_scope_exit-c++.h"
                   "context-c++.h"
                   "iccp-c++.h"
                   "image-c++.h"
                   "image_reader-c++.h"
                   "image_writer-c++.h"
                   "io_common-c++.h"
                   "log-c++.h"
                   "meta_data-c++.h"
                   "palette-c++.h"
                   "codec_info-c++.h"
                   "read_features-c++.h"
                   "read_options-c++.h"
                   "resolution-c++.h"
                   "sail-c++.h"
                   "source_image-c++.h"
                   "write_features-c++.h"
                   "write_options-c++.h"
                   "utils-c++.h")

set_target_properties(sail-c++ PROPERTIES
                               VERSION "0.5.0"
                               SOVERSION 0
                               PUBLIC_HEADER "${PUBLIC_HEADERS}")

sail_enable_pch(TARGET sail-c++ HEADER sail-c++.h)

# Definitions, includes, link
#
target_link_libraries(sail-c++ PUBLIC sail)

# pkg-config integration
#
get_target_property(VERSION sail-c++ VERSION)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/libsail-c++.pc.in"
                "${CMAKE_CURRENT_BINARY_DIR}/libsail-c++.pc" @ONLY)

# Installation
#
install(TARGETS sail-c++
        EXPORT SailC++Targets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-c++")

# Install development packages
#
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libsail-c++.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

sail_install_cmake_config(TARGET "SailC++" VERSION "${VERSION}")
