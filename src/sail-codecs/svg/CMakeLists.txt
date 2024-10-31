find_library(RESVG_LIBRARY resvg)
find_path(RESVG_INCLUDE_DIRS resvg.h)

# Check the ReSVG version is fresh enough
#
if (RESVG_LIBRARY AND RESVG_INCLUDE_DIRS)
    cmake_push_check_state(RESET)
        set(CMAKE_REQUIRED_INCLUDES ${RESVG_INCLUDE_DIRS})
        set(CMAKE_REQUIRED_LIBRARIES ${RESVG_LIBRARY})

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
            set(CMAKE_REQUIRED_INCLUDES ${RESVG_INCLUDE_DIRS})

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

        set(SVG_LIBRARY ${RESVG_LIBRARY})
        set(SVG_INCLUDE_DIRS ${RESVG_INCLUDE_DIRS})

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
    # Don't use SAIL_CODEC_SVG_REQUIRED_OPTION as it requires CMake 3.18
    #
    find_path(NANOSVG_INCLUDE_DIRS nanosvg/nanosvg.h)

    if (NANOSVG_INCLUDE_DIRS)
        set(SVG_INCLUDE_DIRS ${NANOSVG_INCLUDE_DIRS})
    else()
        if (SAIL_CODEC_SVG_REQUIRED_OPTION STREQUAL "REQUIRED")
            message(FATAL_ERROR "SVG: Missing dependencies (ReSVG or NanoSVG)")
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

    # NanoSVG requires the math library
    #
    find_library(MATH_LIBRARY m)

    if (MATH_LIBRARY)
        target_link_libraries(${SAIL_CODEC_TARGET} PRIVATE ${MATH_LIBRARY})
    endif()
endif()
