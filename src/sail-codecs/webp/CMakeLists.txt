find_library(WEBP_RELEASE_LIBRARY NAMES webp)
find_library(WEBP_DEBUG_LIBRARY NAMES webpd webp)
find_library(WEBP_DEMUX_RELEASE_LIBRARY NAMES webpdemux)
find_library(WEBP_DEMUX_DEBUG_LIBRARY NAMES webpdemuxd webpdemux)
find_path(WEBP_INCLUDE_DIRS webp/decode.h)

if ((NOT WEBP_RELEASE_LIBRARY AND NOT WEBP_DEBUG_LIBRARY) OR (NOT WEBP_DEMUX_RELEASE_LIBRARY AND NOT WEBP_DEMUX_DEBUG_LIBRARY) OR NOT WEBP_INCLUDE_DIRS)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_library(webp_RELEASE_LIBRARY NAMES webp)
# find_library(webp_DEBUG_LIBRARY NAMES webpd webp)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES $<$<CONFIG:Release>:${webp_RELEASE_LIBRARY}> $<$<CONFIG:Debug>:${webp_DEBUG_LIBRARY}>)
#
# Same to webpdemux.
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,webp,webpd" "find_library,webpdemux,webpdemuxd" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME webp
            SOURCES helpers.h helpers.c webp.c
            ICON webp.png
            DEPENDENCY_INCLUDE_DIRS ${WEBP_INCLUDE_DIRS}
            DEPENDENCY_LIBS optimized ${WEBP_RELEASE_LIBRARY} debug ${WEBP_DEBUG_LIBRARY} optimized ${WEBP_DEMUX_RELEASE_LIBRARY} debug ${WEBP_DEMUX_DEBUG_LIBRARY})
