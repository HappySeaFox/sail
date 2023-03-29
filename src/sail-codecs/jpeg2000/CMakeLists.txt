find_package(Jasper ${SAIL_CODEC_JPEG2000_REQUIRED_OPTION})

if (NOT JASPER_FOUND)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_package(Jasper)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${JASPER_LIBRARIES})
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_dependency,Jasper,\${JASPER_LIBRARIES}" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME jpeg2000
            SOURCES helpers.h helpers.c jpeg2000.c
            ICON jpeg2000.png
            DEPENDENCY_INCLUDE_DIRS ${JASPER_INCLUDE_DIR}
            DEPENDENCY_LIBS ${JASPER_LIBRARIES})
