find_package(PkgConfig ${SAIL_CODEC_VIDEO_REQUIRED_OPTION})

if (NOT PkgConfig_FOUND)
    return()
endif()

pkg_check_modules(FFMPEG ${SAIL_CODEC_VIDEO_REQUIRED_OPTION}
                  libavformat libavcodec libswscale libswresample libavutil)

if (NOT FFMPEG_FOUND)
    return()
endif()

# For static builds, export dependencies
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "pkg_config,libavformat libavcodec libswscale libswresample libavutil")

# Static FFmpeg builds pull in platform specific system libraries
#
if (WIN32)
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "link_libraries,secur32 ncrypt crypt32 bcrypt mfuuid strmiids")
elseif (APPLE)
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "link_frameworks,AudioToolbox VideoToolbox CoreMedia CoreVideo CoreFoundation")
elseif (ANDROID)
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "link_libraries,mediandk android")
endif()

set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME video
            SOURCES helpers.h helpers.c io_src.h io_src.c video.c
            ICON video.png
            DEPENDENCY_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${FFMPEG_LINK_LIBRARIES})
