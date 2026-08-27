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

# FFmpeg decompresses some streams and container headers with zlib, bzip2, and lzma
# (PNG streams in Matroska, compressed MOV headers etc.). Static FFmpeg builds don't pull
# these libraries in automatically, so find and link them explicitly.
#
include(SelectLibraryConfigurations)

set(COMPRESSION_PREFIXES       ZLIB      BZIP2            LZMA)
set(COMPRESSION_NAMES_RELEASE  "z zlib"  "bz2 bzip2"      "lzma")
set(COMPRESSION_NAMES_DEBUG    "z zlibd" "bz2d bz2 bzip2" "lzmad lzma")

list(LENGTH COMPRESSION_PREFIXES length)
math(EXPR COMPRESSION_PREFIXES_LAST_INDEX "${length} - 1")

foreach (i RANGE ${COMPRESSION_PREFIXES_LAST_INDEX})
    list(GET COMPRESSION_PREFIXES      ${i} var_prefix)
    list(GET COMPRESSION_NAMES_RELEASE ${i} names_release)
    list(GET COMPRESSION_NAMES_DEBUG   ${i} names_debug)

    string(REPLACE " " ";" names_release_as_list ${names_release})
    string(REPLACE " " ";" names_debug_as_list   ${names_debug})

    # The libraries are optional. FFmpeg may be built without them.
    #
    if (SAIL_VCPKG)
        find_library("VIDEO_${var_prefix}_LIBRARY_RELEASE" NAMES ${names_release_as_list} PATHS "${AVCODEC_INCLUDE_DIR}/../lib"       NO_DEFAULT_PATH)
        find_library("VIDEO_${var_prefix}_LIBRARY_DEBUG"   NAMES ${names_debug_as_list}   PATHS "${AVCODEC_INCLUDE_DIR}/../debug/lib" NO_DEFAULT_PATH)
    else()
        find_library("VIDEO_${var_prefix}_LIBRARY_RELEASE" NAMES ${names_release_as_list})
        find_library("VIDEO_${var_prefix}_LIBRARY_DEBUG"   NAMES ${names_debug_as_list})
    endif()

    select_library_configurations(VIDEO_${var_prefix})

    if (VIDEO_${var_prefix}_LIBRARY)
        list(APPEND COMPRESSION_LIBRARIES ${VIDEO_${var_prefix}_LIBRARY})
        set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,${names_release},${names_debug}")
    endif()
endforeach()

# For static builds, export dependencies
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,avformat,avformat")
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,avcodec,avcodec")
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,swresample,swresample")
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,avutil,avutil")
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,swscale,swscale")

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
            DEPENDENCY_INCLUDE_DIRS ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR} ${SWRESAMPLE_INCLUDE_DIR}
            DEPENDENCY_LIBS ${AVFORMAT_LIBRARY} ${AVCODEC_LIBRARY} ${SWRESAMPLE_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY}
                            ${COMPRESSION_LIBRARIES})
