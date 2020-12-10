# Intended to be included by applications. Enables ASAN for executables if possible.
#
macro(sail_enable_asan)
    cmake_parse_arguments(SAIL_ASAN "" "TARGET" "" ${ARGN})

    # Enable ASAN on the specified target (with specific compilers) in DEV mode only
    #
    if (SAIL_DEV)
        if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
            message("ASAN is enabled")
            target_compile_options(${SAIL_ASAN_TARGET} PRIVATE "-fsanitize=address")
            target_link_libraries(${SAIL_ASAN_TARGET} PRIVATE "-fsanitize=address")
        else()
            message("ASAN is not supported with this compiler.")
        endif()
    endif()
endmacro()
