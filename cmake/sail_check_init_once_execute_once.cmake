# Intended to be included by SAIL.
#
function(sail_check_init_once_execute_once)
    cmake_push_check_state(RESET)
        check_c_source_compiles(
        "
            #include \"Windows.h\"
            static BOOL CALLBACK OnceHandler(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContext)
            {
                (void)InitOnce;
                (void)Parameter;
                (void)lpContext;
            }

            int main(int argc, char *argv[]) {
                InitOnceExecuteOnce(NULL, OnceHandler, NULL, NULL);
                return 0;
            }
        "
        SAIL_HAVE_INIT_ONCE_EXECUTE_ONCE
        )
    cmake_pop_check_state()

    if (NOT SAIL_HAVE_INIT_ONCE_EXECUTE_ONCE)
        message(FATAL_ERROR "Currently selected ${CMAKE_C_COMPILER_ID} compiler doesn't support InitOnceExecuteOnce().")
    endif()
endfunction()
