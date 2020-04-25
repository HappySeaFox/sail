# Options
#
option(SAIL_COLORED_OUTPUT "Enable colored console output on supported platforms" ON)

# Export options to the parent cmake file to print statistics
#
set(SAIL_COLORED_OUTPUT ${SAIL_COLORED_OUTPUT} PARENT_SCOPE)

add_library(sail-common
                src/image.c
                src/io.c
                src/io_file.c
                src/meta_entry_node.c
                src/log.c
                src/read_features.c
                src/read_options.c
                src/utils.c
                src/write_features.c
                src/write_options.c)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS "")
list(APPEND PUBLIC_HEADERS "src/common.h")
list(APPEND PUBLIC_HEADERS "src/error.h")
list(APPEND PUBLIC_HEADERS "src/export.h")
list(APPEND PUBLIC_HEADERS "src/image.h")
list(APPEND PUBLIC_HEADERS "src/io_common.h")
list(APPEND PUBLIC_HEADERS "src/meta_entry_node.h")
list(APPEND PUBLIC_HEADERS "src/read_features.h")
list(APPEND PUBLIC_HEADERS "src/read_options.h")
list(APPEND PUBLIC_HEADERS "src/log.h")
list(APPEND PUBLIC_HEADERS "src/sail-common.h")
list(APPEND PUBLIC_HEADERS "src/utils.h")
list(APPEND PUBLIC_HEADERS "src/write_features.h")
list(APPEND PUBLIC_HEADERS "src/write_options.h")

set_target_properties(sail-common PROPERTIES
                                  VERSION "0.5.0"
                                  SOVERSION 0
                                  PUBLIC_HEADER "${PUBLIC_HEADERS}")

# Definitions, includes, link
#
if (SAIL_COLORED_OUTPUT)
    target_compile_definitions(sail-common PRIVATE SAIL_COLORED_OUTPUT=1)
endif()
target_include_directories(sail-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Installation
#
install(TARGETS sail-common LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail")
