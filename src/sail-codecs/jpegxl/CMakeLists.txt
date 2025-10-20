include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

find_path(JPEGXL_INCLUDE_DIR jxl/decode.h)

# Find dependencies for static builds as libjxl doesn't provide CMake configs
# to find them automatically.
#
# See also https://github.com/microsoft/vcpkg/issues/47882
#
set(VAR_PREFIXES JPEGXL     JPEGXL_THREADS     JPEGXL_CMS     HWY BROTLI_COMMON       BROTLI_DEC       BROTLI_ENC       LCMS2)
set(LIBS_SHARED  jxl        jxl_threads        jxl_cms        hwy brotlicommon        brotlidec        brotlienc        lcms2)
set(LIBS_STATIC  jxl-static jxl_threads-static jxl_cms-static hwy brotlicommon-static brotlidec-static brotlienc-static lcms2)

list(LENGTH VAR_PREFIXES length)
math(EXPR VAR_PREFIXES_LAST_INDEX "${length} - 1")

foreach(i RANGE ${VAR_PREFIXES_LAST_INDEX})
    list(GET VAR_PREFIXES ${i} var_prefix)
    list(GET LIBS_SHARED  ${i} lib_shared)
    list(GET LIBS_STATIC  ${i} lib_static)

    # Some libraries are required, some not. Using SAIL_CODEC_JPEGXL_REQUIRED_OPTION overcomplicates
    # the searching mechanism.
    #
    if (SAIL_VCPKG)
        find_library("${var_prefix}_LIBRARY_RELEASE" NAMES ${lib_shared} ${lib_static} PATHS "${JPEGXL_INCLUDE_DIR}/../lib"       NO_DEFAULT_PATH)
        find_library("${var_prefix}_LIBRARY_DEBUG"   NAMES ${lib_shared} ${lib_static} PATHS "${JPEGXL_INCLUDE_DIR}/../debug/lib" NO_DEFAULT_PATH)
    else()
        find_library("${var_prefix}_LIBRARY_RELEASE" NAMES ${lib_shared} ${lib_static})
        find_library("${var_prefix}_LIBRARY_DEBUG"   NAMES ${lib_shared} ${lib_static})
    endif()

    select_library_configurations(${var_prefix})

    find_package_handle_standard_args(${var_prefix}
        REQUIRED_VARS
            JPEGXL_INCLUDE_DIR
            ${var_prefix}_LIBRARY)
endforeach()

if (NOT JPEGXL_INCLUDE_DIR OR NOT JPEGXL_LIBRARY OR NOT JPEGXL_THREADS_LIBRARY OR
        NOT HWY_LIBRARY OR NOT BROTLI_COMMON_LIBRARY OR NOT BROTLI_DEC_LIBRARY OR NOT BROTLI_ENC_LIBRARY)
    if (SAIL_CODEC_JPEGXL_REQUIRED_OPTION STREQUAL "REQUIRED")
        message(FATAL_ERROR "JPEGXL: Missing dependencies")
    else()
        return()
    endif()
endif()

if (JPEGXL_CMS_LIBRARY AND NOT LCMS2_LIBRARY)
    nessage(FATAL_ERROR "LCMS2 library is required")
endif()

# libjxl requires JXL_STATIC_DEFINE for static builds
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
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,jxl jxl-static,jxl jxl-static")
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,jxl_threads jxl_threads-static,jxl_threads jxl_threads-static")

if (JPEGXL_CMS_LIBRARY)
    set(JPEGXL_CMS_LIBRARY_LINK ${JPEGXL_CMS_LIBRARY})
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,jxl_cms jxl_cms-static,jxl_cms jxl_cms-static")
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,lcms2,lcms2")
endif()

set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,brotlidec brotlidec-static,brotlidec brotlidec-static")
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,brotlienc brotlienc-static,brotlienc brotlienc-static")
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,brotlicommon brotlicommon-static,brotlicommon brotlicommon-static")
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,hwy,hwy")

set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} PARENT_SCOPE)

if (NOT BUILD_SHARED_LIBS)
    find_library(MATH_LIBRARY m)

    if (MATH_LIBRARY)
        set(EXTRA_LINK stdc++ ${MATH_LIBRARY})
    else()
        set(EXTRA_LINK stdc++)
    endif()
endif()


# Common codec configuration
#
sail_codec(NAME jpegxl
            SOURCES helpers.h helpers.c jpegxl.c memory.h memory.c
            ICON jpegxl.png
            DEPENDENCY_COMPILE_DEFINITIONS ${JXL_STATIC_DEFINE}
            DEPENDENCY_INCLUDE_DIRS ${JPEGXL_INCLUDE_DIR}
            DEPENDENCY_LIBS ${JPEGXL_LIBRARY} ${JPEGXL_THREADS_LIBRARY} ${JPEGXL_CMS_LIBRARY_LINK}
                            ${BROTLI_DEC_LIBRARY} ${BROTLI_ENC_LIBRARY} ${BROTLI_COMMON_LIBRARY} ${HWY_LIBRARY} ${LCMS2_LIBRARY}
                            ${EXTRA_LINK})
