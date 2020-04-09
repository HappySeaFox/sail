add_library(sail
                src/ini.c
                src/plugin.c
                src/sail.c)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS "")
list(APPEND PUBLIC_HEADERS "src/plugin.h")
list(APPEND PUBLIC_HEADERS "src/sail.h")

set_target_properties(sail PROPERTIES
                           VERSION "0.5.0"
                           SOVERSION 0
                           PUBLIC_HEADER "${PUBLIC_HEADERS}")

# Definitions, includes, link
#
target_include_directories(sail PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(sail PRIVATE INI_STOP_ON_FIRST_ERROR=1)
target_link_libraries(sail PUBLIC sail-common)

# pkg-config integration
#
get_target_property(VERSION sail VERSION)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/libsail.pc.in"
                "${CMAKE_CURRENT_BINARY_DIR}/libsail.pc" @ONLY)

# Installation
#
install(TARGETS sail LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libsail.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
