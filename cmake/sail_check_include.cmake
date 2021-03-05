# Intended to be included by SAIL.
#
macro(sail_check_include)
    set(SAIL_CHECK_INCLUDE ${ARGV0})

    # Build "SAIL_HAVE_STDIO_H"
    #
    string(TOUPPER ${SAIL_CHECK_INCLUDE} SAIL_HAVE_THIS_H)
    string(REGEX REPLACE "[/\\.;]" "_" SAIL_HAVE_THIS_H ${SAIL_HAVE_THIS_H})
    set(SAIL_HAVE_THIS_H SAIL_HAVE_${SAIL_HAVE_THIS_H})

    check_include_files(${SAIL_CHECK_INCLUDE} ${SAIL_HAVE_THIS_H})

    if (NOT ${${SAIL_HAVE_THIS_H}})
        message(FATAL_ERROR "${SAIL_CHECK_INCLUDE} include file is not found. Please check the required development packages are installed.")
    endif()
endmacro()
