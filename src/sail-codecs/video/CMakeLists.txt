# Find FFmpeg AV_LIBRARIES using CMake (no pkg-config)
#
find_path(AVCODEC_INCLUDE_DIR    NAMES libavcodec/avcodec.h)
find_path(AVFORMAT_INCLUDE_DIR   NAMES libavformat/avformat.h)
find_path(AVUTIL_INCLUDE_DIR     NAMES libavutil/avutil.h)
find_path(SWSCALE_INCLUDE_DIR    NAMES libswscale/swscale.h)
find_path(SWRESAMPLE_INCLUDE_DIR NAMES libswresample/swresample.h)

find_library(AVCODEC_LIBRARY    NAMES avcodec)
find_library(AVFORMAT_LIBRARY   NAMES avformat)
find_library(AVUTIL_LIBRARY     NAMES avutil)
find_library(SWSCALE_LIBRARY    NAMES swscale)
find_library(SWRESAMPLE_LIBRARY NAMES swresample)

if (NOT AVCODEC_INCLUDE_DIR OR NOT AVCODEC_LIBRARY
        OR NOT AVFORMAT_INCLUDE_DIR OR NOT AVFORMAT_LIBRARY
        OR NOT AVUTIL_INCLUDE_DIR OR NOT AVUTIL_LIBRARY
        OR NOT SWSCALE_INCLUDE_DIR OR NOT SWSCALE_LIBRARY)
    return()
endif()

# For static builds, export dependencies
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,avcodec,avcodec")
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,avformat,avformat")
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,avutil,avutil")
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,swscale,swscale")
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,swresample,swresample")
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME video
            SOURCES helpers.h helpers.c io_src.h io_src.c video.c
            ICON video.png
            DEPENDENCY_INCLUDE_DIRS ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR} ${SWRESAMPLE_INCLUDE_DIR}
            DEPENDENCY_LIBS ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY} ${SWRESAMPLE_LIBRARY})
