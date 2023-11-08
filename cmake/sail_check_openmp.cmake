# Intended to be included by SAIL.
#
function(sail_check_openmp)
    find_package(OpenMP)

    if (OpenMP_FOUND)
        if (MSVC)
            set(SAIL_OPENMP_FLAGS "-openmp:llvm" CACHE INTERNAL "")
        else()
            set(SAIL_OPENMP_FLAGS ${OpenMP_C_FLAGS} CACHE INTERNAL "")
        endif()

        set(SAIL_OPENMP_INCLUDE_DIRS ${OpenMP_C_INCLUDE_DIRS} CACHE INTERNAL "")
        set(SAIL_OPENMP_LIB_NAMES    ${OpenMP_C_LIB_NAMES}    CACHE INTERNAL "")

        # Try to compile a sample program to make sure the compiler
        # supports at least OpenMP 3.0 with unsigned integers in for loops.
        #
        cmake_push_check_state(RESET)
            set(CMAKE_REQUIRED_FLAGS     ${SAIL_OPENMP_FLAGS})
            set(CMAKE_REQUIRED_INCLUDES  ${SAIL_OPENMP_INCLUDE_DIRS})
            set(CMAKE_REQUIRED_LIBRARIES ${SAIL_OPENMP_LIB_NAMES})

            check_c_source_compiles(
            "
                #include <stdio.h>

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
    else()
        set(SAIL_HAVE_OPENMP_DISPLAY OFF CACHE INTERNAL "")
    endif()
endfunction()
