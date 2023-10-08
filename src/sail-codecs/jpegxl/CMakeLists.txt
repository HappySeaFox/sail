find_library(JPEGXL_LIBRARY jxl                 NAMES jxl jxl-static                 ${SAIL_CODEC_JPEGXL_REQUIRED_OPTION})
find_library(JPEGXL_THREADS_LIBRARY jxl_threads NAMES jxl_threads jxl_threads-static ${SAIL_CODEC_JPEGXL_REQUIRED_OPTION})
find_path(JPEGXL_INCLUDE_DIRS jxl/decode.h                                           ${SAIL_CODEC_JPEGXL_REQUIRED_OPTION})

if (NOT JPEGXL_LIBRARY OR NOT JPEGXL_THREADS_LIBRARY OR NOT JPEGXL_INCLUDE_DIRS)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_library(jxl_RELEASE_LIBRARY NAMES jxl jxl-static)
# find_library(jxl_DEBUG_LIBRARY NAMES jxl jxl-static)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES $<$<CONFIG:Release>:${jxl_RELEASE_LIBRARY}> $<$<CONFIG:Debug>:${jxl_DEBUG_LIBRARY}>)
# find_library(jxl_threads_RELEASE_LIBRARY NAMES jxl_threads jxl_threads-static)
# find_library(jxl_threads_DEBUG_LIBRARY NAMES jxl_threads jxl_threads-static)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES $<$<CONFIG:Release>:${jxl_threads_RELEASE_LIBRARY}> $<$<CONFIG:Debug>:${jxl_threads_DEBUG_LIBRARY}>)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,jxl jxl-static,jxl jxl-static" PARENT_SCOPE)
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,jxl_threads jxl_threads-static,jxl_threads jxl_threads-static" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME jpegxl
            SOURCES helpers.h helpers.c jpegxl.c memory.h memory.c
            ICON jpegxl.png
            DEPENDENCY_INCLUDE_DIRS ${JPEGXL_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${JPEGXL_LIBRARY} ${JPEGXL_THREADS_LIBRARY})