# Intended to be included by SAIL libraries to enable precompiled headers with CMake >= 3.16
#
macro(sail_enable_pch)
    cmake_parse_arguments(SAIL_PCH "" "TARGET;HEADER" "" ${ARGN})

    # Enable precompiled headers on the specified target
    #
    if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
        target_precompile_headers(${SAIL_PCH_TARGET} PRIVATE ${SAIL_PCH_HEADER})
    endif()
endmacro()
