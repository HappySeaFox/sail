add_library(sail-c++)

target_sources(sail-c++
    PRIVATE
        FILE_SET private_headers TYPE HEADERS
        FILES
            at_scope_exit.h
            suppress_begin.h
            suppress_c4251.h
            suppress_end.h
        FILE_SET private_cxx_modules TYPE CXX_MODULES
        FILES
            abstract_io_adapter.cpp
            utils_private.cpp
            utils_private.h
    PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        FILES
            abstract_io.cpp
            arbitrary_data.cpp
            codec_info.cpp
            common.cpp
            common_flags.cpp
            compression_level.cpp
            context.cpp
            conversion_options.cpp
            flags.cpp
            iccp.cpp
            image.h
            image_input.h
            image_output.h
            io_base.cpp
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
                               SOVERSION ${PROJECT_VERSION_MAJOR})

sail_enable_asan(TARGET sail-c++)

sail_enable_pch(TARGET sail-c++ HEADER sail-c++.h)

if (SAIL_INSTALL_PDB)
    sail_install_pdb(TARGET sail-c++)
endif()

# Definitions, includes, link
#
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
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-c++"
        FILE_SET cxx_modules DESTINATION "${CMAKE_INSTALL_LIBDIR}/share/sail-c++/modules")

install(EXPORT SailC++Targets
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/share/sail-c++/modules"
        CXX_MODULES_DIRECTORY .)

# Install development packages
#
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sail-c++.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

sail_install_cmake_config(TARGET "SailC++" FOLDER "sailc++" VERSION "${VERSION}")
