# Enables Position Independent Code (PIC) on an interface library.
#
# Usage:
#   sail_enable_pic(INTERFACE_LIB sail-common-flags)
#
macro(sail_enable_pic)
    cmake_parse_arguments(SAIL_PIC "" "INTERFACE_LIB" "" ${ARGN})

    if (NOT TARGET ${SAIL_PIC_INTERFACE_LIB})
        message(FATAL_ERROR "Interface library '${SAIL_PIC_INTERFACE_LIB}' not found.")
    endif()

    # Set Position Independent Code property
    #
    set_target_properties(${SAIL_PIC_INTERFACE_LIB} PROPERTIES
        INTERFACE_POSITION_INDEPENDENT_CODE ON
    )
endmacro()
