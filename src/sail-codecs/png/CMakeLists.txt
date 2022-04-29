find_package(PNG)

if (NOT PNG_FOUND)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_dependency(PNG REQUIRED)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES PNG::PNG)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_dependency,PNG,PNG::PNG" PARENT_SCOPE)

# Check for APNG features
#
cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_INCLUDES ${PNG_INCLUDE_DIRS})
    set(CMAKE_REQUIRED_LIBRARIES ${PNG_LIBRARIES})

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

# Used in .codec.info
#
if (HAVE_APNG)
    set(PNG_CODEC_INFO_EXTENSION_APNG   ";apng")
    set(PNG_CODEC_INFO_FEATURE_ANIMATED ";ANIMATED")
endif()

# Common codec configuration
#
sail_codec(NAME png
            SOURCES helpers.h helpers.c io.h io.c png.c
            ICON png.png
            DEPENDENCY_INCLUDE_DIRS ${PNG_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${PNG_LIBRARIES})
