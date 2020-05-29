# Check for APNG features
#
cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_INCLUDES "${libpng16_INCLUDE_DIRS}")
    set(CMAKE_REQUIRED_LIBRARIES PkgConfig::libpng16)

    check_c_source_compiles(
        "
        #include <stdio.h>
        #include <png.h>

        int main(int argc, char *argv[]) {
            png_get_first_frame_is_hidden(NULL, NULL);
            return 0;
        }
    "
    HAVE_APNG
    )
cmake_pop_check_state()

if (HAVE_APNG)
    set(PLUGIN_INFO_EXTENSION_APNG   ";apng")
    set(PLUGIN_INFO_FEATURE_ANIMATED ";ANIMATED")
endif()

# Common plugin configuration
#
sail_plugin(TARGET png SOURCES helpers.c io.c png.c PKGCONFIG libpng16)
