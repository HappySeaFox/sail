# Intended to enable exception handling model on an interface library for Windows MSVC builds.
#
# Usage:
#   sail_windows_enable_exception_handling(INTERFACE_LIB sail-common-flags)
#
macro(sail_windows_enable_exception_handling)
    cmake_parse_arguments(SAIL_EH "" "INTERFACE_LIB" "" ${ARGN})

    if (NOT TARGET ${SAIL_EH_INTERFACE_LIB})
        message(FATAL_ERROR "sail_windows_enable_exception_handling: Interface library '${SAIL_EH_INTERFACE_LIB}' not found.")
    endif()

    if (MSVC)
        target_compile_options(${SAIL_EH_INTERFACE_LIB} INTERFACE "/EHsc")
    endif()
endmacro()
