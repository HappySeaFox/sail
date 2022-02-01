# Intended to be included by SAIL libraries to install their debug PDB files
#
macro(sail_install_pdb)
    cmake_parse_arguments(SAIL_INSTALL_PDB "" "TARGET;RENAME" "" ${ARGN})

    if (MSVC AND BUILD_SHARED_LIBS)
        if (SAIL_INSTALL_PDB_RENAME)
            install(FILES $<TARGET_PDB_FILE:${SAIL_INSTALL_PDB_TARGET}> DESTINATION bin RENAME ${SAIL_INSTALL_PDB_RENAME} OPTIONAL)
        else()
            install(FILES $<TARGET_PDB_FILE:${SAIL_INSTALL_PDB_TARGET}> DESTINATION bin OPTIONAL)
        endif()
    endif()
endmacro()
