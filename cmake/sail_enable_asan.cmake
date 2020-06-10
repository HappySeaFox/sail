# Intended to be included by applications. Enables ASAN for executables if possible.
#
macro(sail_enable_asan)
    cmake_parse_arguments(SAIL_ASAN "" "TARGET" "" ${ARGN})

    # Enable ASAN on the specified target if possible. SAIL_ASAN_FLAGS is set in the main cmake file
    #
    if (SAIL_ASAN_FLAGS)
        if (CMAKE_COMPILER_IS_GNUCC)
            target_compile_options("${SAIL_ASAN_TARGET}" PRIVATE "${SAIL_ASAN_FLAGS}")
            target_link_libraries("${SAIL_ASAN_TARGET}" "${SAIL_ASAN_FLAGS}")
        else()
            message("ASAN is not supported with this compiler.")
        endif()
    endif()
endmacro()
