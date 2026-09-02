find_package(PkgConfig ${SAIL_CODEC_VIDEO_REQUIRED_OPTION})

if (NOT PkgConfig_FOUND)
    return()
endif()

# Without this pkg-config only searches its own default directories and misses FFmpeg
# installed into a prefix passed through CMAKE_PREFIX_PATH, like a vcpkg tree.
#
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)

pkg_check_modules(FFMPEG ${SAIL_CODEC_VIDEO_REQUIRED_OPTION}
                  libavformat libavcodec libswscale libswresample libavutil)

if (NOT FFMPEG_FOUND)
    return()
endif()

# For static builds, export dependencies
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "pkg_config,libavformat libavcodec libswscale libswresample libavutil")

# Static FFmpeg builds pull in platform specific system libraries. pkg-config does not
# report them, and the codec itself needs them just as much as a client does: the exported
# entries below only reach the link through sail-codecs, which exists when codecs are
# combined into an archive.
#
if (WIN32)
    set(VIDEO_SYSTEM_LIBRARIES secur32 ncrypt crypt32 bcrypt mfuuid strmiids)

    list(JOIN VIDEO_SYSTEM_LIBRARIES " " VIDEO_SYSTEM_LIBRARIES_JOINED)
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "link_libraries,${VIDEO_SYSTEM_LIBRARIES_JOINED}")
elseif (APPLE)
    set(VIDEO_SYSTEM_FRAMEWORKS AudioToolbox VideoToolbox CoreMedia CoreVideo CoreFoundation)

    list(JOIN VIDEO_SYSTEM_FRAMEWORKS " " VIDEO_SYSTEM_FRAMEWORKS_JOINED)
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "link_frameworks,${VIDEO_SYSTEM_FRAMEWORKS_JOINED}")

    foreach (VIDEO_SYSTEM_FRAMEWORK IN LISTS VIDEO_SYSTEM_FRAMEWORKS)
        list(APPEND VIDEO_SYSTEM_LIBRARIES "-framework ${VIDEO_SYSTEM_FRAMEWORK}")
    endforeach()
elseif (ANDROID)
    set(VIDEO_SYSTEM_LIBRARIES mediandk android)

    list(JOIN VIDEO_SYSTEM_LIBRARIES " " VIDEO_SYSTEM_LIBRARIES_JOINED)
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "link_libraries,${VIDEO_SYSTEM_LIBRARIES_JOINED}")
endif()

set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME video
            SOURCES helpers.h helpers.c io_src.h io_src.c video.c
            ICON video.png
            DEPENDENCY_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${FFMPEG_LINK_LIBRARIES} ${VIDEO_SYSTEM_LIBRARIES})
