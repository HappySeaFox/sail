# Enables TSAN on an interface library if supported.
# We try to compile and RUN a test program with -fsanitize=thread to check if TSAN
# is available and working.
#
# Usage:
#   sail_enable_tsan(INTERFACE_LIB sail-common-flags)
#
macro(sail_enable_tsan)
    cmake_parse_arguments(SAIL_TSAN "" "INTERFACE_LIB" "" ${ARGN})

    if (NOT TARGET ${SAIL_TSAN_INTERFACE_LIB})
        message(FATAL_ERROR "Interface library '${SAIL_TSAN_INTERFACE_LIB}' not found.")
    endif()

    if (SAIL_TSAN)
        if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
            cmake_push_check_state(RESET)
                set(CMAKE_REQUIRED_FLAGS "-fsanitize=thread")
                set(CMAKE_REQUIRED_LIBRARIES "-fsanitize=thread")

                check_c_source_runs(
                    "
                    int main(int argc, char *argv[]) {
                        return 0;
                    }
                    "
                    SAIL_HAVE_TSAN
                )
            cmake_pop_check_state()
        endif()

        if (SAIL_HAVE_TSAN)
            target_compile_options(${SAIL_TSAN_INTERFACE_LIB} INTERFACE "-fsanitize=thread")
            target_link_options(${SAIL_TSAN_INTERFACE_LIB} INTERFACE "-fsanitize=thread")
        endif()
    endif()
endmacro()
