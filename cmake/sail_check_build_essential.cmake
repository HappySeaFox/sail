include(CheckIncludeFiles)

# Intended to be included by SAIL.
#
macro(sail_check_include)
    set(SAIL_CHECK_INCLUDE "${ARGV0}")

    # Build "HAVE_STDIO_H"
    #
    string(TOUPPER "${SAIL_CHECK_INCLUDE}" HAVE_THIS_H)
    string(REGEX REPLACE "[/\\.;]" "_" HAVE_THIS_H "${HAVE_THIS_H}")
    set(HAVE_THIS_H "HAVE_${HAVE_THIS_H}")

    check_include_files("${SAIL_CHECK_INCLUDE}" "${HAVE_THIS_H}")

    if (NOT "${${HAVE_THIS_H}}")
        message(FATAL_ERROR "${SAIL_CHECK_INCLUDE} include file is not found. Please check the required development packages are installed.")
    endif()
endmacro()
