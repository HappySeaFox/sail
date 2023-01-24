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
        check_c_source_compiles(
        "
            int main(int argc, char *argv[]) {
                __builtin_bswap32(0);
                return 0;
            }
        "
        SAIL_HAVE_BUILTIN_BSWAP32
        )
        check_c_source_compiles(
        "
            int main(int argc, char *argv[]) {
                __builtin_bswap64(0);
                return 0;
            }
        "
        SAIL_HAVE_BUILTIN_BSWAP64
        )
    cmake_pop_check_state()
endfunction()
