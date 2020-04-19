add_library(sail-c++
                src/context-c++.cpp
                src/image_reader-c++.cpp
                src/image-c++.cpp
                src/plugin_info-c++.cpp)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS "")
list(APPEND PUBLIC_HEADERS "src/at_scope_exit-c++.h")
list(APPEND PUBLIC_HEADERS "src/context-c++.h")
list(APPEND PUBLIC_HEADERS "src/plugin_info-c++.h")
list(APPEND PUBLIC_HEADERS "src/image_reader-c++.h")
list(APPEND PUBLIC_HEADERS "src/image-c++.h")

set_target_properties(sail-c++ PROPERTIES
                               VERSION "0.3.1"
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
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libsail-c++.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
