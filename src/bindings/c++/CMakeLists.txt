add_library(sail-c++
                abstract_io-c++.h
                abstract_io_adapter-c++.cpp
                abstract_io_adapter-c++.h
                arbitrary_data-c++.h
                at_scope_exit-c++.h
                codec_info-c++.cpp
                codec_info-c++.h
                context-c++.cpp
                context-c++.h
                conversion_options-c++.cpp
                conversion_options-c++.h
                iccp-c++.cpp
                iccp-c++.h
                image-c++.cpp
                image-c++.h
                image_input-c++.cpp
                image_input-c++.h
                image_output-c++.cpp
                image_output-c++.h
                io_file-c++.cpp
                io_file-c++.h
                log-c++.cpp
                log-c++.h
                meta_data-c++.cpp
                meta_data-c++.h
                palette-c++.cpp
                palette-c++.h
                read_features-c++.cpp
                read_features-c++.h
                read_options-c++.cpp
                read_options-c++.h
                resolution-c++.cpp
                resolution-c++.h
                sail-c++.h
                source_image-c++.cpp
                source_image-c++.h
                utils-c++.cpp
                utils-c++.h
                write_features-c++.cpp
                write_features-c++.h
                write_options-c++.cpp
                write_options-c++.h)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS "abstract_io-c++.h"
                   "arbitrary_data-c++.h"
                   "at_scope_exit-c++.h"
                   "context-c++.h"
                   "conversion_options-c++.h"
                   "iccp-c++.h"
                   "image-c++.h"
                   "image_input-c++.h"
                   "image_output-c++.h"
                   "io_file-c++.cpp"
                   "io_file-c++.h"
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
                               VERSION "0.7.1"
                               SOVERSION 0
                               PUBLIC_HEADER "${PUBLIC_HEADERS}")

sail_enable_pch(TARGET sail-c++ HEADER sail-c++.h)

sail_install_pdb(TARGET sail-c++)

# Definitions, includes, link
#
target_include_directories(sail-c++ PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if (MSVC)
    target_compile_options(sail-c++ PRIVATE "/EHsc")
endif()

target_link_libraries(sail-c++ PUBLIC sail-common sail sail-manip)

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

sail_install_cmake_config(TARGET "SailC++" FOLDER "sailc++" VERSION "${VERSION}")
