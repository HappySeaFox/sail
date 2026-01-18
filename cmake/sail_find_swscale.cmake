# Intended to be included by SAIL.
# Finds libswscale library for sail-manip and processes the MODE option.
#
# Arguments:
#   MODE - AUTO, ON, or OFF
#
# Sets:
#   SAIL_MANIP_SWSCALE_ENABLED - ON or OFF
#
macro(sail_find_swscale MODE)
    set(SAIL_MANIP_SWSCALE_ENABLED OFF)
    set(SAIL_MANIP_SWSCALE_ENABLED_DISPLAY "OFF")

    if (${MODE} STREQUAL "ON" OR ${MODE} STREQUAL "AUTO")
        find_path(SAIL_SWSCALE_INCLUDE_DIR NAMES libswscale/swscale.h)
        find_library(SAIL_SWSCALE_LIBS NAMES swscale)

        if (SAIL_SWSCALE_INCLUDE_DIR AND SAIL_SWSCALE_LIBS)
            message(STATUS "Using libswscale for pixel format conversion in sail-manip")
            set(SAIL_MANIP_SWSCALE_ENABLED ON)
            set(SAIL_MANIP_SWSCALE_ENABLED_DISPLAY "ON")
        else()
            if (${MODE} STREQUAL "ON")
                message(FATAL_ERROR "libswscale not found but SAIL_MANIP_USE_SWSCALE=ON requires it")
            else()
                message(STATUS "libswscale not found, not using swscale for pixel format conversion")
            endif()
        endif()
    elseif (NOT ${MODE} STREQUAL "OFF")
        message(FATAL_ERROR "SAIL_MANIP_USE_SWSCALE must be AUTO, ON, or OFF (got: ${MODE})")
    else()
        set(SAIL_MANIP_SWSCALE_ENABLED_DISPLAY "OFF (forced)")
    endif()
endmacro()
