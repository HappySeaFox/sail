# Intended to be included by SAIL libraries to enable _XOPEN_SOURCE on UNIX
#
macro(sail_enable_xopen_source)
    cmake_parse_arguments(SAIL_XOPEN_SOURCE "" "TARGET;VERSION" "" ${ARGN})

    # Enable _XOPEN_SOURCE on the specified target
    #
    if (UNIX)
        target_compile_definitions(${SAIL_XOPEN_SOURCE_TARGET} PRIVATE _XOPEN_SOURCE=${SAIL_XOPEN_SOURCE_VERSION})
    endif()
endmacro()
