# Intended to be included by applications. Enables ASAN for executables if possible.
#
macro(sail_enable_asan)
    cmake_parse_arguments(SAIL_ASAN "" "TARGET" "" ${ARGN})

    # Enable ASAN on the specified target in DEV mode only
    #
    if (SAIL_DEV)
        if (CMAKE_COMPILER_IS_GNUCC)
            target_compile_options("${SAIL_ASAN_TARGET}" PRIVATE "-fsanitize=address,leak")
            target_link_libraries("${SAIL_ASAN_TARGET}" "-fsanitize=address,leak")
        else()
            message("ASAN is not supported with this compiler.")
        endif()
    endif()
endmacro()
