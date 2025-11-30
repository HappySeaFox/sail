# Intended to set CRT options for Windows MSVC builds on an interface library.
#
# Usage:
#   sail_windows_set_crt(INTERFACE_LIB sail-common-flags STATIC_CRT ON)  # Use /MT or /MTd
#   sail_windows_set_crt(INTERFACE_LIB sail-common-flags STATIC_CRT OFF) # Use /MD or /MDd
#
macro(sail_windows_set_crt)
    cmake_parse_arguments(SAIL_CRT "" "INTERFACE_LIB;STATIC_CRT" "" ${ARGN})

    if (NOT TARGET ${SAIL_CRT_INTERFACE_LIB})
        message(FATAL_ERROR "sail_windows_set_crt: Interface library '${SAIL_CRT_INTERFACE_LIB}' not found.")
    endif()

    if (WIN32 AND MSVC)
        if (SAIL_CRT_STATIC_CRT)
            target_compile_options(${SAIL_CRT_INTERFACE_LIB} INTERFACE
                $<$<CONFIG:Debug>:/MTd>
                $<$<NOT:$<CONFIG:Debug>>:/MT>
            )
        else()
            target_compile_options(${SAIL_CRT_INTERFACE_LIB} INTERFACE
                $<$<CONFIG:Debug>:/MDd>
                $<$<NOT:$<CONFIG:Debug>>:/MD>
            )
        endif()
    endif()
endmacro()
