find_package(WEBP ${SAIL_CODEC_WEBP_REQUIRED_OPTION})

if (NOT WEBP_FOUND)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_dependency(WEBP REQUIRED)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES WebP::webp WebP::webpdecoder WebP::webpdemux)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_dependency,WEBP,WebP::webp WebP::webpdecoder WebP::webpdemux" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME webp
            SOURCES helpers.h helpers.c webp.c
            ICON webp.png
            DEPENDENCY_LIBS WebP::webp WebP::webpdecoder WebP::webpdemux)
