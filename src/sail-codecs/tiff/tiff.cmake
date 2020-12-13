macro(sail_find_dependencies)
    find_package(TIFF)

    if (NOT TIFF_FOUND)
        return()
    endif()

    set(sail_tiff_include_dirs ${TIFF_INCLUDE_DIRS})
    set(sail_tiff_libs ${TIFF_LIBRARIES})
endmacro()

macro(sail_codec_post_add)
    cmake_push_check_state(RESET)
        set(CMAKE_REQUIRED_INCLUDES ${sail_tiff_include_dirs})

        check_c_source_compiles(
            "
            #include <tiff.h>

            int main(int argc, char *argv[]) {
                int compression = COMPRESSION_WEBP;
                return 0;
            }
        "
        HAVE_TIFF_41
        )
    cmake_pop_check_state()

    if (HAVE_TIFF_41)
        # We compile libtiff w/o WEBP support on Windows
        if (WIN32)
            set(CODEC_INFO_COMPRESSIONS_41 "ZSTD")
        else()
            set(CODEC_INFO_COMPRESSIONS_41 "WEBP;ZSTD")
        endif()
    endif()

    if (HAVE_TIFF_41)
        target_compile_definitions(${TARGET} PRIVATE HAVE_TIFF_41)
    endif()
endmacro()
