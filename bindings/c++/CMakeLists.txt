add_library(sail-c++
                src/plugin_info-c++.cpp
                src/sail-c++.cpp)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS "")
list(APPEND PUBLIC_HEADERS "src/plugin_info-c++.h")
list(APPEND PUBLIC_HEADERS "src/sail-c++.h")

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
