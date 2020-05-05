# Intended to be included by applications. Enables ASAN for executables if possible.
#
macro(sail_enable_asan)
    cmake_parse_arguments(SAIL_ASAN "" "TARGET" "" ${ARGN})

    # Enable ASAN on the specified target if possible
    #
    if (SAIL_ASAN)
        if (CMAKE_COMPILER_IS_GNUCC)
            target_compile_options("${SAIL_ASAN_TARGET}" PRIVATE "${SAIL_ASAN}")
            target_link_libraries("${SAIL_ASAN_TARGET}" "${SAIL_ASAN}")
        else()
            message("ASAN is not supported on this platform.")
        endif()
    endif()
endmacro()
