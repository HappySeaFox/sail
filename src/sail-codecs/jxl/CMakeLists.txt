find_library(JXL_LIBRARY jxl            ${SAIL_CODEC_JXL_REQUIRED_OPTION})
find_path(JXL_INCLUDE_DIRS jxl/decode.h ${SAIL_CODEC_JXL_REQUIRED_OPTION})

if (NOT JXL_LIBRARY OR NOT JXL_INCLUDE_DIRS)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_library(jxl_RELEASE_LIBRARY NAMES jxl)
# find_library(jxl_DEBUG_LIBRARY NAMES jxl)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES $<$<CONFIG:Release>:${jxl_RELEASE_LIBRARY}> $<$<CONFIG:Debug>:${jxl_DEBUG_LIBRARY}>)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,jxl,jxl" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME jxl
            SOURCES helpers.h helpers.c jxl.c memory.h memory.c
            ICON jxl.png
            DEPENDENCY_INCLUDE_DIRS ${JXL_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${JXL_LIBRARY})
