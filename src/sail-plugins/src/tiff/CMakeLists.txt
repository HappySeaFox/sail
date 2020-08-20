cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_INCLUDES "${libtiff-4_INCLUDE_DIRS}")

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
        set(PLUGIN_INFO_COMPRESSIONS_41 "ZSTD")
    else()
        set(PLUGIN_INFO_COMPRESSIONS_41 "WEBP;ZSTD")
    endif()
endif()

# Common plugin configuration
#
if (WIN32)
    set(TIFF_DEPS libtiff-4 libjpeg zlib libzstd)
else()
    set(TIFF_DEPS libtiff-4)
endif()

sail_plugin(TARGET tiff SOURCES helpers.c io.c tiff.c PKGCONFIG ${TIFF_DEPS})

if (HAVE_TIFF_41)
    target_compile_definitions(tiff PRIVATE HAVE_TIFF_41)
endif()
