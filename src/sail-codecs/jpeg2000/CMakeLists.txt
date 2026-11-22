find_package(OpenJPEG ${SAIL_CODEC_JPEG2000_REQUIRED_OPTION})

if (NOT OpenJPEG_FOUND)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_package(OpenJPEG)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${OPENJPEG_LIBRARIES})
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_dependency,OpenJPEG,\${OPENJPEG_LIBRARIES}" PARENT_SCOPE)

# OpenJPEG before 2.5.4 fails to write ICC profiles
# https://github.com/uclouvain/openjpeg/pull/1574
#
cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_INCLUDES ${OPENJPEG_INCLUDE_DIRS})

    check_c_source_compiles(
        "
        #include <openjpeg.h>

        #if (OPJ_VERSION_MAJOR > 2) || (OPJ_VERSION_MAJOR == 2 && OPJ_VERSION_MINOR > 5) \
            || (OPJ_VERSION_MAJOR == 2 && OPJ_VERSION_MINOR == 5 && OPJ_VERSION_BUILD >= 4)
        #else
            #error OpenJPEG is too old to save ICC profiles
        #endif

        int main(int argc, char *argv[]) {
            return 0;
        }
    "
    HAVE_OPENJPEG_ICCP
    )
cmake_pop_check_state()

# Used in .codec.info
#
if (HAVE_OPENJPEG_ICCP)
    set(JPEG2000_CODEC_INFO_FEATURE_ICCP ";ICCP")
endif()

# Common codec configuration
#
sail_codec(NAME jpeg2000
            SOURCES helpers.h helpers.c io_dest.h io_dest.c io_src.h io_src.c jpeg2000.c
            ICON jpeg2000.png
            DEPENDENCY_INCLUDE_DIRS ${OPENJPEG_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${OPENJPEG_LIBRARIES})
