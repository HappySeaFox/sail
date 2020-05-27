# Check for JPEG ICC functions that were added in libjpeg-turbo-1.5.90
#
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
    HAVE_JPEG_ICC
)

if (HAVE_JPEG_ICC)
    set(PLUGIN_INFO_FEATURE_ICC ";ICC")
endif()

# Common plugin configuration
#
sail_plugin(TARGET jpeg SOURCES jpeg.c helpers.c io_dest.c io_src.c PKGCONFIG libjpeg)

if (HAVE_JPEG_ICC)
    target_compile_definitions(jpeg PRIVATE HAVE_JPEG_ICC)
endif()
