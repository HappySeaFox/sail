add_library(sail-c++
                abstract_io.h
                abstract_io_adapter.cpp
                abstract_io_adapter.h
                arbitrary_data.h
                at_scope_exit.h
                codec_info.cpp
                codec_info.h
                compression_level.cpp
                compression_level.h
                context.cpp
                context.h
                conversion_options.cpp
                conversion_options.h
                iccp.cpp
                iccp.h
                image.cpp
                image.h
                image_input.cpp
                image_input.h
                image_output.cpp
                image_output.h
                io_base.cpp
                io_base.h
                io_base_private.h
                io_expanding_buffer.cpp
                io_expanding_buffer.h
                io_file.cpp
                io_file.h
                io_memory.cpp
                io_memory.h
                load_features.cpp
                load_features.h
                load_options.cpp
                load_options.h
                log.cpp
                log.h
                meta_data.cpp
                meta_data.h
                ostream.h
                ostream.cpp
                palette.cpp
                palette.h
                resolution.cpp
                resolution.h
                sail-c++.h
                save_features.cpp
                save_features.h
                save_options.cpp
                save_options.h
                source_image.cpp
                source_image.h
                special_properties.h
                suppress_begin.h
                suppress_c4251.h
                suppress_end.h
                tuning.h
                utils.cpp
                utils.h
                utils_private.cpp
                utils_private.h
                variant.cpp
                variant.h)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS abstract_io.h
                   arbitrary_data.h
                   at_scope_exit.h
                   codec_info.h
                   context.h
                   conversion_options.h
                   compression_level.h
                   iccp.h
                   image.h
                   image_input.h
                   image_output.h
                   io_base.h
                   io_expanding_buffer.h
                   io_file.h
                   io_memory.h
                   load_features.h
                   load_options.h
                   log.h
                   meta_data.h
                   ostream.h
                   palette.h
                   resolution.h
                   sail-c++.h
                   save_features.h
                   save_options.h
                   source_image.h
                   special_properties.h
                   suppress_begin.h
                   suppress_c4251.h
                   suppress_end.h
                   tuning.h
                   utils.h
                   variant.h)

set_target_properties(sail-c++ PROPERTIES
                               VERSION ${PROJECT_VERSION}
                               SOVERSION ${PROJECT_VERSION_MAJOR}
                               PUBLIC_HEADER "${PUBLIC_HEADERS}")

# Definitions, includes, link
#
target_link_libraries(sail-c++ PRIVATE $<BUILD_INTERFACE:sail-common-flags>)

sail_enable_pch(TARGET sail-c++ HEADER sail-c++.h)

if (SAIL_WINDOWS_INSTALL_PDB)
    sail_windows_install_pdb(TARGET sail-c++)
endif()

target_include_directories(sail-c++ PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)

if (MSVC)
    target_compile_options(sail-c++ PRIVATE "/EHsc")
endif()

target_link_libraries(sail-c++ PUBLIC sail-common sail sail-manip)

# pkg-config integration
#
get_target_property(VERSION sail-c++ VERSION)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/sail-c++.pc.in"
                "${CMAKE_CURRENT_BINARY_DIR}/sail-c++.pc" @ONLY)

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
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sail-c++.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

sail_install_cmake_config(TARGET "SailC++" FOLDER "sailc++" VERSION "${VERSION}")
