# Intended to be included by SAIL.
#
function(sail_check_openmp)
    # This may require setting OpenMP_ROOT on macOS
    #
    find_package(OpenMP COMPONENTS C)

    if (OpenMP_FOUND)
        # We want OpenMP 3.0 to support unsigned integers in loops.
        # The default OpenMP implementation in MSVC 2022 still supports 2.0,
        # so switch to the LLVM option with OpenMP 3.1.
        #
        # Don't use "if (MSVC)" as some compilers (Clang-CL) may simulate MSVC,
        # but don't support /openmp:llvm.
        #
        if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
            set(SAIL_OPENMP_FLAGS "/openmp:llvm" CACHE INTERNAL "")
        else()
            set(SAIL_OPENMP_FLAGS ${OpenMP_C_FLAGS} CACHE INTERNAL "")
        endif()

        set(SAIL_OPENMP_INCLUDE_DIRS ${OpenMP_C_INCLUDE_DIRS} CACHE INTERNAL "")

        # Build a list of direct paths to libraries. This is needed on macOS with brew specifically
        # as the installed libomp version lays in /usr/local/opt/libomp and is not globally visible.
        #
        foreach(lib IN LISTS OpenMP_C_LIB_NAMES)
            set(SAIL_OPENMP_LIBS ${SAIL_OPENMP_LIBS} "${OpenMP_${lib}_LIBRARY}" CACHE INTERNAL "")
        endforeach()

        # Try to compile a sample program to make sure the compiler
        # supports at least OpenMP 3.0 with unsigned integers in for loops.
        #
        cmake_push_check_state(RESET)
            set(CMAKE_REQUIRED_FLAGS     ${SAIL_OPENMP_FLAGS})
            set(CMAKE_REQUIRED_INCLUDES  ${SAIL_OPENMP_INCLUDE_DIRS})
            set(CMAKE_REQUIRED_LIBRARIES ${SAIL_OPENMP_LIBS})

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
    endif()

    if (SAIL_HAVE_OPENMP)
        set(SAIL_HAVE_OPENMP_DISPLAY ON CACHE INTERNAL "")
    else()
        set(SAIL_HAVE_OPENMP_DISPLAY OFF CACHE INTERNAL "")
    endif()
endfunction()
