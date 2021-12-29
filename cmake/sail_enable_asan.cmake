# Intended to be included by applications. Enables ASAN for executables if possible.
#
macro(sail_enable_asan)
    cmake_parse_arguments(SAIL_ASAN "" "TARGET" "" ${ARGN})

    # Enable ASAN on the specified target in DEV mode only. We try to compile and run
    # a test program with -fsanitize=address as ASAN may be accessible in MSVC 2019 on Windows 7,
    # but the resulting program fails to run due to mising symbols in kernel32.
    #
    if (SAIL_DEV)
        cmake_push_check_state(RESET)
            set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")

            # GCC and Clang require to link ASAN as well
            if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
                set(CMAKE_REQUIRED_LIBRARIES "-fsanitize=address")
            endif()

            check_c_source_runs(
                "
                int main(int argc, char *argv[]) {
                    return 0;
                }
                "
                SAIL_HAVE_ASAN
            )
        cmake_pop_check_state()

        if (SAIL_HAVE_ASAN)
            message("${SAIL_ASAN_TARGET}: ASAN is enabled")

            target_compile_options(${SAIL_ASAN_TARGET} PRIVATE "-fsanitize=address")

            # GCC and Clang require to link ASAN as well
            if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
                target_link_libraries(${SAIL_ASAN_TARGET} PRIVATE "-fsanitize=address")
            endif()
        else()
            message("${SAIL_ASAN_TARGET}: ASAN is not supported with this compiler.")
        endif()
    endif()
endmacro()
