macro(sail_find_dependencies)
    find_library(WEBP_RELEASE_LIBRARY NAMES webp)
    find_library(WEBP_DEBUG_LIBRARY NAMES webpd webp)
    find_library(WEBP_DEMUX_RELEASE_LIBRARY NAMES webpdemux)
    find_library(WEBP_DEMUX_DEBUG_LIBRARY NAMES webpdemuxd webpdemux)
    find_path(WEBP_INCLUDE_DIRS webp/decode.h)

    if ((NOT WEBP_RELEASE_LIBRARY AND NOT WEBP_DEBUG_LIBRARY) OR (NOT WEBP_DEMUX_RELEASE_LIBRARY AND NOT WEBP_DEMUX_DEBUG_LIBRARY) OR NOT WEBP_INCLUDE_DIRS)
        return()
    endif()

    set(WEBP_LIBRARY optimized ${WEBP_RELEASE_LIBRARY} debug ${WEBP_DEBUG_LIBRARY})
    set(WEBP_DEMUX_LIBRARY optimized ${WEBP_DEMUX_RELEASE_LIBRARY} debug ${WEBP_DEMUX_DEBUG_LIBRARY})

    set(sail_webp_include_dirs ${WEBP_INCLUDE_DIRS})
    set(sail_webp_libs ${WEBP_LIBRARY} ${WEBP_DEMUX_LIBRARY})

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
endmacro()
