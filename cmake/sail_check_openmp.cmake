# Intended to be included by SAIL.
#
function(sail_check_openmp)
    find_package(OpenMP)

    if (OpenMP_FOUND)
        # Try to compile a sample program as some compilers may still
        # fail to use OpenMP
        #
        cmake_push_check_state(RESET)
            set(CMAKE_REQUIRED_FLAGS     ${OpenMP_C_FLAGS})
            set(CMAKE_REQUIRED_INCLUDES  ${OpenMP_C_INCLUDE_DIRS})
            set(CMAKE_REQUIRED_LIBRARIES ${OpenMP_C_LIBRARIES})

            check_c_source_compiles(
            "
                #include <stdio.h>
                #include <omp.h>

                int main(int argc, char *argv[]) {
                    unsigned i;
                    #pragma omp parallel for
                    for (i = 0; i < 10; i++) {
                        printf(\"%d\", i);
                    }

                    return 0;
                }
            "
            SAIL_HAVE_OPENMP
            )
        cmake_pop_check_state()

        if (SAIL_HAVE_OPENMP)
            set(SAIL_HAVE_OPENMP_DISPLAY ON CACHE INTERNAL "")
        else()
            set(SAIL_HAVE_OPENMP_DISPLAY OFF CACHE INTERNAL "")
        endif()
    endif()
endfunction()
