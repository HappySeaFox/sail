# Enable as many warnings as possible on an interface library.
#
# Usage:
#   sail_enable_warnings(INTERFACE_LIB sail-common-flags)
#
macro(sail_enable_warnings)
    cmake_parse_arguments(SAIL_WARNINGS "" "INTERFACE_LIB" "" ${ARGN})

    if (NOT TARGET ${SAIL_WARNINGS_INTERFACE_LIB})
        message(FATAL_ERROR "Interface library '${SAIL_WARNINGS_INTERFACE_LIB}' not found.")
    endif()

    # Add warnings to the interface library
    #
    if (MSVC)
        target_compile_options(${SAIL_WARNINGS_INTERFACE_LIB} INTERFACE /W4)
    elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
        target_compile_options(${SAIL_WARNINGS_INTERFACE_LIB} INTERFACE
            $<$<COMPILE_LANGUAGE:C>:-Wall -Wextra>
            $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra>
        )

        if (SAIL_DEV)
            target_compile_options(${SAIL_WARNINGS_INTERFACE_LIB} INTERFACE
                $<$<COMPILE_LANGUAGE:C>:-pedantic>
                $<$<COMPILE_LANGUAGE:CXX>:-pedantic>
            )
        endif()
    endif()
endmacro()
