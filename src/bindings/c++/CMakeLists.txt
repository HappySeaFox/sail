add_library(sail-c++
                src/context-c++.cpp
                src/iccp-c++.cpp
                src/image-c++.cpp
                src/image_reader-c++.cpp
                src/image_writer-c++.cpp
                src/io_common-c++.cpp
                src/log-c++.cpp
                src/palette-c++.cpp
                src/plugin_info-c++.cpp
                src/read_features-c++.cpp
                src/read_options-c++.cpp
                src/source_image-c++.cpp
                src/write_features-c++.cpp
                src/write_options-c++.cpp
                src/utils-c++.cpp)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS "")
list(APPEND PUBLIC_HEADERS "src/at_scope_exit-c++.h")
list(APPEND PUBLIC_HEADERS "src/context-c++.h")
list(APPEND PUBLIC_HEADERS "src/iccp-c++.h")
list(APPEND PUBLIC_HEADERS "src/image-c++.h")
list(APPEND PUBLIC_HEADERS "src/image_reader-c++.h")
list(APPEND PUBLIC_HEADERS "src/image_writer-c++.h")
list(APPEND PUBLIC_HEADERS "src/io_common-c++.h")
list(APPEND PUBLIC_HEADERS "src/log-c++.h")
list(APPEND PUBLIC_HEADERS "src/palette-c++.h")
list(APPEND PUBLIC_HEADERS "src/plugin_info-c++.h")
list(APPEND PUBLIC_HEADERS "src/read_features-c++.h")
list(APPEND PUBLIC_HEADERS "src/read_options-c++.h")
list(APPEND PUBLIC_HEADERS "src/sail-c++.h")
list(APPEND PUBLIC_HEADERS "src/source_image-c++.h")
list(APPEND PUBLIC_HEADERS "src/write_features-c++.h")
list(APPEND PUBLIC_HEADERS "src/write_options-c++.h")
list(APPEND PUBLIC_HEADERS "src/utils-c++.h")

set_target_properties(sail-c++ PROPERTIES
                               VERSION "0.5.0"
                               SOVERSION 0
                               PUBLIC_HEADER "${PUBLIC_HEADERS}")

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
install(TARGETS sail-c++ LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-c++")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libsail-c++.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
