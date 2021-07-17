macro(sail_find_dependencies)
    find_package(JPEG)

    if (NOT JPEG_FOUND)
        return()
    endif()

    set(sail_jpeg_include_dirs ${JPEG_INCLUDE_DIR})
    set(sail_jpeg_libs ${JPEG_LIBRARIES})

    # This will add the following CMake rules to the CMake config for static builds so a client
    # application links against the required dependencies:
    #
    # find_dependency(JPEG REQUIRED)
    # set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES JPEG::JPEG)
    #
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "JPEG,JPEG::JPEG" PARENT_SCOPE)
endmacro()

macro(sail_codec_post_add)
    # Check for JPEG ICC functions that were added in libjpeg-turbo-1.5.90
    #
    cmake_push_check_state(RESET)
        set(CMAKE_REQUIRED_INCLUDES ${sail_jpeg_include_dirs})
        set(CMAKE_REQUIRED_LIBRARIES ${sail_jpeg_libs})

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
        SAIL_HAVE_JPEG_ICCP
        )
    cmake_pop_check_state()

    if (SAIL_HAVE_JPEG_ICCP)
        set(CODEC_INFO_FEATURE_ICCP ";ICCP")
    endif()

    if (SAIL_HAVE_JPEG_ICCP)
        target_compile_definitions(${TARGET} PRIVATE SAIL_HAVE_JPEG_ICCP)
    endif()

    # Check for libjpeg-turbo
    #
    cmake_push_check_state(RESET)
        set(CMAKE_REQUIRED_INCLUDES ${sail_jpeg_include_dirs})
        set(CMAKE_REQUIRED_LIBRARIES ${sail_jpeg_libs})

        check_c_source_compiles(
            "
            #include <stdio.h>
            #include <jpeglib.h>

            int main(int argc, char *argv[]) {
                J_COLOR_SPACE ext = JCS_EXT_RGB;
                return 0;
            }
        "
        SAIL_HAVE_JPEG_JCS_EXT
        )
    cmake_pop_check_state()

    if (SAIL_HAVE_JPEG_JCS_EXT)
        target_compile_definitions(${TARGET} PRIVATE SAIL_HAVE_JPEG_JCS_EXT)
        set(SAIL_JPEG_CODEC_INFO_WRITE_EXT "BPP24-RGB;")
    endif()
endmacro()
