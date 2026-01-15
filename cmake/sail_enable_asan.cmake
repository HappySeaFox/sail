# Enables ASAN on an interface library if supported.
# We try to compile and RUN a test program with -fsanitize=address as ASAN may be
# accessible in MSVC 2019 on Windows 7, but the resulting program may fail to run
# due to missing symbols in kernel32.
#
# Usage:
#   sail_enable_asan(INTERFACE_LIB sail-common-flags)
#
macro(sail_enable_asan)
    cmake_parse_arguments(SAIL_ASAN "" "INTERFACE_LIB" "" ${ARGN})

    if (NOT TARGET ${SAIL_ASAN_INTERFACE_LIB})
        message(FATAL_ERROR "Interface library '${SAIL_ASAN_INTERFACE_LIB}' not found.")
    endif()

    if (SAIL_ASAN)
        if (NOT MSVC OR MSVC_TOOLSET_VERSION GREATER 141)
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
        endif()

        if (SAIL_HAVE_ASAN)
            target_compile_options(${SAIL_ASAN_INTERFACE_LIB} INTERFACE "-fsanitize=address")

            # GCC and Clang require to link ASAN as well
            if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
                target_link_options(${SAIL_ASAN_INTERFACE_LIB} INTERFACE "-fsanitize=address")
            endif()
        endif()
    endif()
endmacro()
