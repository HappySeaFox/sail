find_package(JPEG)

if (NOT JPEG_FOUND)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_dependency(JPEG REQUIRED)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES JPEG::JPEG)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_dependency,JPEG,JPEG::JPEG" PARENT_SCOPE)

# Check for JPEG ICC functions that were added in libjpeg-turbo-1.5.90
#
cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_INCLUDES ${JPEG_INCLUDE_DIR})
    set(CMAKE_REQUIRED_LIBRARIES ${JPEG_LIBRARIES})

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

# Used in .codec.info
#
if (HAVE_JPEG_ICCP)
    set(JPEG_CODEC_INFO_FEATURE_ICCP ";ICCP")
endif()

# Check for libjpeg-turbo
#
cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_INCLUDES ${JPEG_INCLUDE_DIR})
    set(CMAKE_REQUIRED_LIBRARIES ${JPEG_LIBRARIES})

    check_c_source_compiles(
        "
        #include <stdio.h>
        #include <jpeglib.h>

        int main(int argc, char *argv[]) {
            J_COLOR_SPACE ext = JCS_EXT_RGB;
            return 0;
        }
    "
    HAVE_JPEG_JCS_EXT
    )
cmake_pop_check_state()

# Used in .codec.info
#
if (HAVE_JPEG_JCS_EXT)
    set(JPEG_CODEC_INFO_WRITE_EXT "BPP24-RGB;")
endif()

# Common codec configuration
#
sail_codec(NAME jpeg
            SOURCES helpers.h helpers.c io_dest.h io_dest.c io_src.h io_src.c jpeg.c
            ICON jpeg.png
            DEPENDENCY_INCLUDE_DIRS ${JPEG_INCLUDE_DIR}
            DEPENDENCY_LIBS ${JPEG_LIBRARIES})

# Add compile options to the target based on our findings
#
if (HAVE_JPEG_ICCP)
    target_compile_definitions(${TARGET} PRIVATE SAIL_HAVE_JPEG_ICCP)
endif()

if (HAVE_JPEG_JCS_EXT)
    target_compile_definitions(${TARGET} PRIVATE SAIL_HAVE_JPEG_JCS_EXT)
endif()
