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

# Common codec configuration
#
sail_codec(NAME jpeg2000
            SOURCES helpers.h helpers.c io_dest.h io_dest.c io_src.h io_src.c jpeg2000.c
            ICON jpeg2000.png
            DEPENDENCY_INCLUDE_DIRS ${OPENJPEG_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${OPENJPEG_LIBRARIES})
