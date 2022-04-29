find_package(GIF)

if (NOT GIF_FOUND)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_dependency(GIF REQUIRED)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES GIF::GIF)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_dependency,GIF,GIF::GIF" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME gif
            SOURCES helpers.h helpers.c io.h io.c gif.c
            ICON gif.png
            DEPENDENCY_INCLUDE_DIRS ${GIF_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${GIF_LIBRARIES})
