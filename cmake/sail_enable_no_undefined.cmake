# Enables linker flags to disable undefined symbols on an interface library.
#
# Usage:
#   sail_enable_no_undefined(INTERFACE_LIB sail-common-flags)
#
macro(sail_enable_no_undefined)
    cmake_parse_arguments(SAIL_NO_UNDEFINED "" "INTERFACE_LIB" "" ${ARGN})

    if (NOT TARGET ${SAIL_NO_UNDEFINED_INTERFACE_LIB})
        message(FATAL_ERROR "Interface library '${SAIL_NO_UNDEFINED_INTERFACE_LIB}' not found.")
    endif()

    # Disable undefined symbols for shared/module libraries
    #
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_link_options(${SAIL_NO_UNDEFINED_INTERFACE_LIB} INTERFACE
            -Wl,--no-undefined
        )
    elseif (CMAKE_C_COMPILER_ID MATCHES "Clang")
        target_link_options(${SAIL_NO_UNDEFINED_INTERFACE_LIB} INTERFACE
            -Wl,-undefined,error
        )
    endif()
endmacro()
