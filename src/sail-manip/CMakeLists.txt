add_library(sail-manip
                cmyk.c
                cmyk.h
                conversion_options.c
                conversion_options.h
                convert.c
                convert.h
                manip_common.h
                manip_utils.c
                manip_utils.h
                sail-manip.h
                ycbcr.c
                ycbcr.h
                ycck.c
                ycck.h)

# Build a list of public headers to install
#
set(PUBLIC_HEADERS conversion_options.h
                   convert.h
                   manip_common.h
                   sail-manip.h)

set_target_properties(sail-manip PROPERTIES
                                 VERSION ${PROJECT_VERSION}
                                 SOVERSION ${PROJECT_VERSION_MAJOR}
                                 PUBLIC_HEADER "${PUBLIC_HEADERS}")

sail_enable_asan(TARGET sail-manip)

sail_enable_pch(TARGET sail-manip HEADER sail-manip.h)

if (SAIL_INSTALL_PDB)
    sail_install_pdb(TARGET sail-manip)
endif()

# Definitions, includes, link
#
target_include_directories(sail-manip PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)

if (SAIL_HAVE_OPENMP)
    target_compile_options(sail-manip     PRIVATE ${SAIL_OPENMP_FLAGS})
    target_include_directories(sail-manip PRIVATE ${SAIL_OPENMP_INCLUDE_DIRS})
    target_link_libraries(sail-manip      PRIVATE ${SAIL_OPENMP_LIBS})
endif()

target_link_libraries(sail-manip PUBLIC sail-common)

# pkg-config integration
#
get_target_property(VERSION sail-manip VERSION)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/sail-manip.pc.in"
                "${CMAKE_CURRENT_BINARY_DIR}/sail-manip.pc" @ONLY)

# Installation
#
install(TARGETS sail-manip
        EXPORT SailManipTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-manip")

# Install development packages
#
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sail-manip.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

sail_install_cmake_config(TARGET "SailManip" FOLDER "sailmanip" VERSION "${VERSION}")
