add_library(sail
                codec.c
                codec_bundle.h
                codec_bundle_node.c
                codec_bundle_node.h
                codec_bundle_node_private.c
                codec_bundle_node_private.h
                codec_bundle_private.c
                codec_bundle_private.h
                codec_info.c
                codec_info.h
                codec_info_private.c
                codec_info_private.h
                codec_layout.h
                context.c
                context.h
                context_private.c
                context_private.h
                ini.c
                ini.h
                io_file.c
                io_file.h
                io_mem.c
                io_mem.h
                io_noop.c
                io_noop.h
                sail.h
                sail_advanced.c
                sail_advanced.h
                sail_deep_diver.c
                sail_deep_diver.h
                sail_junior.c
                sail_junior.h
                sail_private.c
                sail_private.h
                sail_technical_diver.c
                sail_technical_diver.h
                sail_technical_diver_private.c
                sail_technical_diver_private.h
                string_node.h
                string_node_private.c
                string_node_private.h
                threading.h
                threading.c)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS "codec_bundle.h"
                   "codec_bundle_node.h"
                   "codec_info.h"
                   "context.h"
                   "sail.h"
                   "sail_advanced.h"
                   "sail_deep_diver.h"
                   "sail_junior.h"
                   "sail_technical_diver.h"
                   "string_node.h")

set_target_properties(sail PROPERTIES
                           VERSION "0.8.0"
                           SOVERSION 0
                           PUBLIC_HEADER "${PUBLIC_HEADERS}")

# setenv
sail_enable_posix_source(TARGET sail VERSION 200112L)

sail_enable_pch(TARGET sail HEADER sail.h)

sail_install_pdb(TARGET sail)

# Definitions, includes, link
#
target_include_directories(sail PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
# Simplify the INIH parser
target_compile_definitions(sail PRIVATE INI_ALLOW_MULTILINE=0 INI_ALLOW_INLINE_COMMENTS=0 INI_CUSTOM_ALLOCATOR=0 INI_STOP_ON_FIRST_ERROR=1 INI_MAX_LINE=500)

if (SAIL_COMBINE_CODECS)
    # Transfer user requirements
    #
    target_link_libraries(sail PRIVATE sail-codecs)
endif()

target_link_libraries(sail PUBLIC sail-common)

if (WIN32)
    sail_check_init_once_execute_once()
endif()

if (UNIX)
    # pthread_once()
    find_package(Threads REQUIRED)
    target_link_libraries(sail PUBLIC ${CMAKE_THREAD_LIBS_INIT})

    # pthread_mutexattr_settype()
    sail_enable_xopen_source(TARGET sail VERSION 500)

    target_link_libraries(sail PRIVATE dl)
endif()

# pkg-config integration
#
get_target_property(VERSION sail VERSION)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/libsail.pc.in"
                "${CMAKE_CURRENT_BINARY_DIR}/libsail.pc" @ONLY)

# Installation
#
install(TARGETS sail
        EXPORT SailTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail/sail")

# Install layouts for debugging codecs
#
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/layout/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail/sail/layout")

# Install development packages
#
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libsail.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

if (SAIL_COMBINE_CODECS)
    set(SAIL_CODECS_FIND_DEPENDENCY "find_dependency(SailCodecs REQUIRED PATHS \${CMAKE_CURRENT_LIST_DIR})")
endif()

sail_install_cmake_config(TARGET "Sail" FOLDER "sail" VERSION "${VERSION}")
