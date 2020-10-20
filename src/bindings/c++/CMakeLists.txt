add_library(sail-c++
                src/context-c++.cpp
                src/iccp-c++.cpp
                src/image-c++.cpp
                src/image_reader-c++.cpp
                src/image_writer-c++.cpp
                src/io_common-c++.cpp
                src/log-c++.cpp
                src/meta_data-c++.cpp
                src/palette-c++.cpp
                src/codec_info-c++.cpp
                src/read_features-c++.cpp
                src/read_options-c++.cpp
                src/resolution-c++.cpp
                src/source_image-c++.cpp
                src/write_features-c++.cpp
                src/write_options-c++.cpp
                src/utils-c++.cpp)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS "src/at_scope_exit-c++.h"
                   "src/context-c++.h"
                   "src/iccp-c++.h"
                   "src/image-c++.h"
                   "src/image_reader-c++.h"
                   "src/image_writer-c++.h"
                   "src/io_common-c++.h"
                   "src/log-c++.h"
                   "src/meta_data-c++.h"
                   "src/palette-c++.h"
                   "src/codec_info-c++.h"
                   "src/read_features-c++.h"
                   "src/read_options-c++.h"
                   "src/resolution-c++.h"
                   "src/sail-c++.h"
                   "src/source_image-c++.h"
                   "src/write_features-c++.h"
                   "src/write_options-c++.h"
                   "src/utils-c++.h")

set_target_properties(sail-c++ PROPERTIES
                               VERSION "0.5.0"
                               SOVERSION 0
                               PUBLIC_HEADER "${PUBLIC_HEADERS}")

sail_enable_pch(TARGET sail-c++ HEADER src/sail-c++.h)

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
