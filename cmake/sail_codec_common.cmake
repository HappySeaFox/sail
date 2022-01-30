# Intended to be used by every private library shared by multiple codecs.
# Sets the library up so that it gets linked correctly in all possible build types.
#
macro(sail_codec_common)
    cmake_parse_arguments(SAIL_CODEC_COMMON "" "NAME" "SOURCES;LINK;VERSION;SOVERSION" ${ARGN})

    if (SAIL_COMBINE_CODECS)
        add_library(${SAIL_CODEC_COMMON_NAME} OBJECT ${SAIL_CODEC_COMMON_SOURCES})

    else()
        add_library(${SAIL_CODEC_COMMON_NAME} STATIC ${SAIL_CODEC_COMMON_SOURCES})
        set_target_properties(${SAIL_CODEC_COMMON_NAME} PROPERTIES
                VERSION "${SAIL_CODEC_COMMON_VERSION}"
                SOVERSION ${SAIL_CODEC_COMMON_SOVERSION})
    endif()

    target_include_directories(${SAIL_CODEC_COMMON_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
    target_link_libraries(${SAIL_CODEC_COMMON_NAME} PRIVATE sail-common ${SAIL_CODEC_COMMON_LINK})
endmacro()
