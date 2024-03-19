find_library(SVG_LIBRARY resvg)
find_path(SVG_INCLUDE_DIRS resvg.h)

# Check the ReSVG version is fresh enough
#
if (SVG_LIBRARY AND SVG_INCLUDE_DIRS)
    cmake_push_check_state(RESET)
        set(CMAKE_REQUIRED_INCLUDES ${SVG_INCLUDE_DIRS})
        set(CMAKE_REQUIRED_LIBRARIES ${SVG_LIBRARY})

        check_c_source_compiles(
            "
            #include <resvg.h>

            int main(int argc, char *argv[]) {
                resvg_options *resvg_options = resvg_options_create();
                return 0;
            }
        "
        HAVE_RESVG_OPTIONS_CREATE
        )
    cmake_pop_check_state()

    if (HAVE_RESVG_OPTIONS_CREATE)
        set(SAIL_RESVG ON)

        cmake_push_check_state(RESET)
            set(CMAKE_REQUIRED_INCLUDES ${SVG_INCLUDE_DIRS})

            check_c_source_compiles(
                "
                #include <resvg.h>

                int main(int argc, char *argv[]) {
                    resvg_fit_to resvg_fit_to = { RESVG_FIT_TO_ORIGINAL, 0 };
                    return 0;
                }
            "
            HAVE_RESVG_FIT_TO
            )
        cmake_pop_check_state()

        # This will add the following CMake rules to the CMake config for static builds so a client
        # application links against the required dependencies:
        #
        # find_library(resvg_RELEASE_LIBRARY NAMES resvg)
        # find_library(resvg_DEBUG_LIBRARY NAMES resvg)
        # set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES $<$<CONFIG:Release>:${resvg_RELEASE_LIBRARY}> $<$<CONFIG:Debug>:${resvg_DEBUG_LIBRARY}>)
        #
        set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,resvg,resvg" PARENT_SCOPE)
    endif()
endif()

# Fall back to NanoSVG
#
if (NOT SAIL_RESVG)
    set(SVG_LIBRARY "")
    # Don't use SAIL_CODEC_SVG_REQUIRED_OPTION as it requires CMake 3.18
    #
    find_path(SVG_INCLUDE_DIRS nanosvg/nanosvg.h)

    if (NOT SVG_INCLUDE_DIRS)
        if (SAIL_CODEC_SVG_REQUIRED_OPTION STREQUAL "REQUIRED")
            message(FATAL_ERROR "SVG: Missing dependencies")
        else()
            return()
        endif()
    endif()
endif()

# Common codec configuration
#
sail_codec(NAME svg
            SOURCES svg.c
            ICON svg.png
            DEPENDENCY_INCLUDE_DIRS ${SVG_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${SVG_LIBRARY})

if (SAIL_RESVG)
    target_compile_definitions(${SAIL_CODEC_TARGET} PRIVATE SAIL_RESVG)

    if (HAVE_RESVG_FIT_TO)
        target_compile_definitions(${SAIL_CODEC_TARGET} PRIVATE SAIL_HAVE_RESVG_FIT_TO)
    endif()
else()
    target_compile_definitions(${SAIL_CODEC_TARGET} PRIVATE SAIL_NANOSVG)
endif()
