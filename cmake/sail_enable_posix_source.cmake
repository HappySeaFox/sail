# Intended to be included by SAIL libraries to enable _POSIX_C_SOURCE on UNIX
#
macro(sail_enable_posix_source)
    cmake_parse_arguments(SAIL_POSIX_SOURCE "" "TARGET;VERSION" "" ${ARGN})

    # Enable _POSIX_C_SOURCE on the specified target
    #
    if (UNIX)
        target_compile_definitions(${SAIL_POSIX_SOURCE_TARGET} PRIVATE _POSIX_C_SOURCE=${SAIL_POSIX_SOURCE_VERSION})
    endif()
endmacro()
