# Check for JPEG ICC functions that were added in libjpeg-turbo-1.5.90
#
cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_INCLUDES "${libjpeg_INCLUDE_DIRS}")
    set(CMAKE_REQUIRED_LIBRARIES PkgConfig::libjpeg)

    check_c_source_compiles(
        "
        #include <stdio.h>
        #include <jpeglib.h>

        int main(int argc, char *argv[]) {
            jpeg_read_icc_profile(NULL, NULL, NULL);
            jpeg_write_icc_profile(NULL, NULL, 0);
            return 0;
        }
    "
    HAVE_JPEG_ICCP
    )
cmake_pop_check_state()

if (HAVE_JPEG_ICCP)
    set(CODEC_INFO_FEATURE_ICCP ";ICCP")
endif()

# Common codec configuration
#
sail_codec(TARGET jpeg SOURCES jpeg.c helpers.c io_dest.c io_src.c PKGCONFIG libjpeg)

if (HAVE_JPEG_ICCP)
    target_compile_definitions(jpeg PRIVATE HAVE_JPEG_ICCP)
endif()
