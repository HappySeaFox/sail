# Options
#
option(SAIL_COLORED_OUTPUT "Enable colored console output on Windows 10+ and Unix platforms." ON)

# Export options to the parent cmake file to print statistics
#
set(SAIL_COLORED_OUTPUT ${SAIL_COLORED_OUTPUT} PARENT_SCOPE)

add_library(sail-common
                common.h
                common_serialize.c
                common_serialize.h
                compiler_specifics.h
                compression_level.h
                compression_level.c
                export.h
                hash_map.c
                hash_map.h
                hash_map_private.h
                iccp.c
                iccp.h
                image.c
                image.h
                io_common.c
                io_common.h
                linked_list_node.c
                linked_list_node.h
                load_features.c
                load_features.h
                load_options.c
                load_options.h
                log.c
                log.h
                memory.c
                memory.h
                meta_data.c
                meta_data.h
                meta_data_node.c
                meta_data_node.h
                palette.c
                palette.h
                pixel.c
                pixel.h
                resolution.c
                resolution.h
                sail-common.h
                save_features.c
                save_features.h
                save_options.c
                save_options.h
                source_image.c
                source_image.h
                status.h
                string_node.c
                string_node.h
                utils.c
                utils.h
                variant.c
                variant.h
                variant_node.c
                variant_node.h)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS common.h
                   common_serialize.h
                   compiler_specifics.h
                   compression_level.h
                   export.h
                   hash_map.h
                   iccp.h
                   image.h
                   io_common.h
                   load_features.h
                   load_options.h
                   log.h
                   memory.h
                   meta_data.h
                   meta_data_node.h
                   palette.h
                   pixel.h
                   resolution.h
                   sail-common.h
                   save_features.h
                   save_options.h
                   source_image.h
                   status.h
                   string_node.h
                   utils.h
                   variant.h
                   variant_node.h)

set_target_properties(sail-common PROPERTIES
                                  VERSION ${PROJECT_VERSION}
                                  SOVERSION ${PROJECT_VERSION_MAJOR}
                                  PUBLIC_HEADER "${PUBLIC_HEADERS}")

# Definitions, includes, link
#
target_link_libraries(sail-common PRIVATE sail-common-flags)

# fileno, mkstemp
sail_enable_posix_c_source(TARGET sail-common VERSION 200809L)

sail_enable_pch(TARGET sail-common HEADER sail-common.h)

if (SAIL_WINDOWS_INSTALL_PDB)
    sail_windows_install_pdb(TARGET sail-common)
endif()

if (SAIL_COLORED_OUTPUT)
    target_compile_definitions(sail-common PRIVATE SAIL_COLORED_OUTPUT=1)
endif()

target_include_directories(sail-common
                            PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
                                   $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
                                   $<INSTALL_INTERFACE:include/sail>)

# pkg-config integration
#
get_target_property(VERSION sail-common VERSION)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/sail-common.pc.in"
                "${CMAKE_CURRENT_BINARY_DIR}/sail-common.pc" @ONLY)

# Installation
#
install(TARGETS sail-common
        EXPORT SailCommonTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-common")

# Install development packages
#
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sail-common.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

sail_install_cmake_config(TARGET "SailCommon" FOLDER "sailcommon" VERSION "${VERSION}")
