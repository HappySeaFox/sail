# Intended to be included by SAIL.
#
function(sail_check_builtin_bswap)
    cmake_push_check_state(RESET)
        check_c_source_compiles(
        "
            int main(int argc, char *argv[]) {
                __builtin_bswap16(0);
                return 0;
            }
        "
        SAIL_HAVE_BUILTIN_BSWAP16
        )
        # This variable is used for displaying the test result with message()
        # in the main CMake file.
        #
        # The cache SAIL_HAVE_BUILTIN_BSWAP16 variable that is created by the test
        # has a value of 1 or an empty string which is not a user friendly value.
        #
        if (SAIL_HAVE_BUILTIN_BSWAP16)
            set(SAIL_HAVE_BUILTIN_BSWAP16_DISPLAY ON CACHE INTERNAL "")
        else()
            set(SAIL_HAVE_BUILTIN_BSWAP16_DISPLAY OFF CACHE INTERNAL "")
        endif()

        check_c_source_compiles(
        "
            int main(int argc, char *argv[]) {
                __builtin_bswap32(0);
                return 0;
            }
        "
        SAIL_HAVE_BUILTIN_BSWAP32
        )
        if (SAIL_HAVE_BUILTIN_BSWAP32)
            set(SAIL_HAVE_BUILTIN_BSWAP32_DISPLAY ON CACHE INTERNAL "")
        else()
            set(SAIL_HAVE_BUILTIN_BSWAP32_DISPLAY OFF CACHE INTERNAL "")
        endif()

        check_c_source_compiles(
        "
            int main(int argc, char *argv[]) {
                __builtin_bswap64(0);
                return 0;
            }
        "
        SAIL_HAVE_BUILTIN_BSWAP64
        )
        if (SAIL_HAVE_BUILTIN_BSWAP64)
            set(SAIL_HAVE_BUILTIN_BSWAP64_DISPLAY ON CACHE INTERNAL "")
        else()
            set(SAIL_HAVE_BUILTIN_BSWAP64_DISPLAY OFF CACHE INTERNAL "")
        endif()
    cmake_pop_check_state()
endfunction()
