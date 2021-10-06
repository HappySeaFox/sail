macro(sail_find_dependencies)
    find_package(PNG)

    if (NOT PNG_FOUND)
        return()
    endif()

    set(sail_png_include_dirs ${PNG_INCLUDE_DIRS})
    set(sail_png_libs ${PNG_LIBRARIES})

    # This will add the following CMake rules to the CMake config for static builds so a client
    # application links against the required dependencies:
    #
    # find_dependency(PNG REQUIRED)
    # set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES PNG::PNG)
    #
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_dependency,PNG,PNG::PNG" PARENT_SCOPE)
endmacro()

macro(sail_codec_post_add)
    # Check for APNG features
    #
    cmake_push_check_state(RESET)
        set(CMAKE_REQUIRED_INCLUDES ${sail_png_include_dirs})
        set(CMAKE_REQUIRED_LIBRARIES ${sail_png_libs})

        check_c_source_compiles(
            "
            #include <stdio.h>
            #include <png.h>

            int main(int argc, char *argv[]) {
                png_get_first_frame_is_hidden(NULL, NULL);
                return 0;
            }
        "
        SAIL_HAVE_APNG
        )
    cmake_pop_check_state()

    if (SAIL_HAVE_APNG)
        set(CODEC_INFO_EXTENSION_APNG   ";apng")
        set(CODEC_INFO_FEATURE_ANIMATED ";ANIMATED")
    endif()
endmacro()
