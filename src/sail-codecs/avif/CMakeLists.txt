find_library(AVIF_LIBRARY avif)
find_path(AVIF_INCLUDE_DIRS avif/avif.h)

if (NOT AVIF_LIBRARY OR NOT AVIF_INCLUDE_DIRS)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_dependency(LIBAVIF REQUIRED)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES avif)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_dependency,LIBAVIF,avif" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME avif
            SOURCES helpers.h helpers.c io.h io.c avif.c
            ICON avif.png
            DEPENDENCY_INCLUDE_DIRS ${AVIF_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${AVIF_LIBRARY})
