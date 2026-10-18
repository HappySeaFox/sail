# Intended to be included by SAIL.
# Finds libswscale library for sail-manip and processes the MODE option.
#
# Arguments:
#   MODE - AUTO, ON, or OFF
#
# Sets:
#   SAIL_MANIP_SWSCALE_ENABLED - ON or OFF
#   SAIL_SWSCALE_INCLUDE_DIRS, SAIL_SWSCALE_LINK_LIBRARIES - when enabled
#
macro(sail_find_swscale MODE)
    set(SAIL_MANIP_SWSCALE_ENABLED OFF)
    set(SAIL_MANIP_SWSCALE_ENABLED_DISPLAY "OFF")

    if (${MODE} STREQUAL "ON" OR ${MODE} STREQUAL "AUTO")
        if (${MODE} STREQUAL "ON")
            set(SAIL_SWSCALE_REQUIRED_OPTION REQUIRED)
        else()
            set(SAIL_SWSCALE_REQUIRED_OPTION QUIET)
        endif()

        # pkg-config only, just like the video codec. sail-manip is linked after the codecs,
        # so its libraries land at the very end of the link line. Plain libswscale and
        # libavutil paths would then sit after the libraries that satisfy them, like zlib
        # or OpenCL, and a static link would fail with undefined references. pkg-config
        # reports the whole dependency closure instead.
        #
        find_package(PkgConfig ${SAIL_SWSCALE_REQUIRED_OPTION})

        if (PkgConfig_FOUND)
            pkg_check_modules(SAIL_SWSCALE ${SAIL_SWSCALE_REQUIRED_OPTION} libswscale libavutil)
        endif()

        if (SAIL_SWSCALE_FOUND)
            message(STATUS "Using libswscale for pixel format conversion in sail-manip")
            set(SAIL_MANIP_SWSCALE_ENABLED ON)
            set(SAIL_MANIP_SWSCALE_ENABLED_DISPLAY "ON")
        else()
            if (${MODE} STREQUAL "ON")
                message(FATAL_ERROR "libswscale not found but SAIL_MANIP_USE_SWSCALE=ON requires it")
            else()
                message(STATUS "libswscale or libavutil not found, not using swscale for pixel format conversion")
            endif()
        endif()
    elseif (NOT ${MODE} STREQUAL "OFF")
        message(FATAL_ERROR "SAIL_MANIP_USE_SWSCALE must be AUTO, ON, or OFF (got: ${MODE})")
    else()
        set(SAIL_MANIP_SWSCALE_ENABLED_DISPLAY "OFF (forced)")
    endif()
endmacro()
