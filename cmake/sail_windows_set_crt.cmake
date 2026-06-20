# Intended to set CRT options for Windows MSVC builds.
#
# Must be called after SAIL_WINDOWS_STATIC_CRT and BUILD_SHARED_LIBS are known,
# but before add_subdirectory() creates compiled targets.
#
# Usage:
#   sail_windows_set_crt(STATIC_CRT ON)   # Use /MT or /MTd for static builds
#   sail_windows_set_crt(STATIC_CRT OFF)  # Use /MD or /MDd
#
macro(sail_windows_set_crt)
    cmake_parse_arguments(SAIL_CRT "" "STATIC_CRT" "" ${ARGN})

    if (WIN32 AND MSVC)
        # Shared DLLs must use the dynamic CRT (/MD). Static builds honor STATIC_CRT.
        #
        if (BUILD_SHARED_LIBS OR NOT SAIL_CRT_STATIC_CRT)
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC runtime library" FORCE)
        else()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "MSVC runtime library" FORCE)
        endif()
    endif()
endmacro()
