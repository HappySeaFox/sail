# Options
#
option(SAIL_COLORED_OUTPUT "Enable colored console output on supported platforms" ON)

# Export options to the parent cmake file to print statistics
#
set(SAIL_COLORED_OUTPUT ${SAIL_COLORED_OUTPUT} PARENT_SCOPE)

add_library(sail-common
                src/common.c
                src/meta_entry_node.c
                src/ini.c
                src/log.c
                src/plugin.c
                src/utils.c)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS "")
list(APPEND PUBLIC_HEADERS "src/common.h")
list(APPEND PUBLIC_HEADERS "src/export.h")
list(APPEND PUBLIC_HEADERS "src/meta_entry_node.h")
list(APPEND PUBLIC_HEADERS "src/log.h")
list(APPEND PUBLIC_HEADERS "src/plugin.h")
list(APPEND PUBLIC_HEADERS "src/utils.h")

set_target_properties(sail-common PROPERTIES
                                  VERSION "0.5.0"
                                  SOVERSION 0
                                  PUBLIC_HEADER "${PUBLIC_HEADERS}")

# Definitions, includes, link
#
if (SAIL_COLORED_OUTPUT)
    target_compile_definitions(sail-common PRIVATE SAIL_COLORED_OUTPUT=1)
endif()
target_compile_definitions(sail-common PRIVATE INI_STOP_ON_FIRST_ERROR=1)
target_include_directories(sail-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# pkg-config integration
#
get_target_property(VERSION sail-common VERSION)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/libsail-common.pc.in"
                "${CMAKE_CURRENT_BINARY_DIR}/libsail-common.pc" @ONLY)

# Installation
#
install(TARGETS sail-common LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libsail-common.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
