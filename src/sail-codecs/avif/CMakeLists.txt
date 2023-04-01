find_package(libavif CONFIG ${SAIL_CODEC_AVIF_REQUIRED_OPTION})

if (NOT libavif_FOUND)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_dependency(LIBAVIF REQUIRED)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES avif)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_dependency,libavif,avif" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME avif
            SOURCES helpers.h helpers.c io.h io.c avif.c
            ICON avif.png
            DEPENDENCY_LIBS avif)
