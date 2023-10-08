find_library(JPEGXL_LIBRARY jxl                 NAMES jxl jxl-static                 ${SAIL_CODEC_JPEGXL_REQUIRED_OPTION})
find_library(JPEGXL_THREADS_LIBRARY jxl_threads NAMES jxl_threads jxl_threads-static ${SAIL_CODEC_JPEGXL_REQUIRED_OPTION})
find_path(JPEGXL_INCLUDE_DIRS jxl/decode.h                                           ${SAIL_CODEC_JPEGXL_REQUIRED_OPTION})

if (NOT JPEGXL_LIBRARY OR NOT JPEGXL_THREADS_LIBRARY OR NOT JPEGXL_INCLUDE_DIRS)
    return()
endif()

# Find dependencies for static builds as libjxl doesn't provide CMake configs
# to find them automatically.
#
find_library(HWY_LIBRARY           NAMES hwy                              ${SAIL_CODEC_JPEGXL_REQUIRED_OPTION})
find_library(BROTLI_COMMON_LIBRARY NAMES brotlicommon brotlicommon-static ${SAIL_CODEC_JPEGXL_REQUIRED_OPTION})
find_library(BROTLI_DEC_LIBRARY    NAMES brotlidec brotlidec-static       ${SAIL_CODEC_JPEGXL_REQUIRED_OPTION})
find_library(LCMS2_LIBRARY         NAMES lcms2                            ${SAIL_CODEC_JPEGXL_REQUIRED_OPTION})

if (NOT HWY_LIBRARY OR NOT BROTLI_COMMON_LIBRARY OR NOT BROTLI_DEC_LIBRARY OR NOT LCMS2_LIBRARY)
    return()
endif()

# For static builds libjxl requires -DJXL_STATIC_DEFINE
#
if (JPEGXL_LIBRARY MATCHES "jxl-static")
    set(JXL_STATIC_DEFINE "JXL_STATIC_DEFINE")
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_library(jxl_RELEASE_LIBRARY NAMES jxl jxl-static)
# find_library(jxl_DEBUG_LIBRARY NAMES jxl jxl-static)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES $<$<CONFIG:Release>:${jxl_RELEASE_LIBRARY}> $<$<CONFIG:Debug>:${jxl_DEBUG_LIBRARY}>)
#
# etc.
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,jxl jxl-static,jxl jxl-static" PARENT_SCOPE)
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,jxl_threads jxl_threads-static,jxl_threads jxl_threads-static" PARENT_SCOPE)

set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,hwy,hwy" PARENT_SCOPE)
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,brotlicommon brotlicommon-static,brotlicommon brotlicommon-static" PARENT_SCOPE)
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,brotlidec brotlidec-static,brotlidec brotlidec-static" PARENT_SCOPE)
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,lcms2,lcms2" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME jpegxl
            SOURCES helpers.h helpers.c jpegxl.c memory.h memory.c
            ICON jpegxl.png
            DEPENDENCY_COMPILE_DEFINITIONS ${JXL_STATIC_DEFINE}
            DEPENDENCY_INCLUDE_DIRS ${JPEGXL_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${BROTLI_COMMON_LIBRARY} ${BROTLI_DEC_LIBRARY} ${HWY_LIBRARY} ${LCMS2_LIBRARY}
                            ${JPEGXL_LIBRARY} ${JPEGXL_THREADS_LIBRARY})
