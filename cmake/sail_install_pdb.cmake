# Intended to be included by SAIL libraries to install their debug PDB files
#
macro(sail_install_pdb)
    cmake_parse_arguments(SAIL_INSTALL_PDB "" "TARGET" "" ${ARGN})

    if (MSVC AND NOT SAIL_STATIC)
        install(FILES $<TARGET_PDB_FILE:${SAIL_INSTALL_PDB_TARGET}> DESTINATION bin OPTIONAL)
    endif()
endmacro()
