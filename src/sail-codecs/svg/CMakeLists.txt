find_library(SVG_LIBRARY resvg)
find_path(SVG_INCLUDE_DIRS resvg.h)

if (NOT SVG_LIBRARY OR NOT SVG_INCLUDE_DIRS)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_library(resvg_RELEASE_LIBRARY NAMES resvg)
# find_library(resvg_DEBUG_LIBRARY NAMES resvg)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES $<$<CONFIG:Release>:${resvg_RELEASE_LIBRARY}> $<$<CONFIG:Debug>:${resvg_DEBUG_LIBRARY}>)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,resvg,resvg" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME svg
            SOURCES svg.c
            ICON svg.png
            DEPENDENCY_INCLUDE_DIRS ${SVG_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${SVG_LIBRARY})
