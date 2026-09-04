find_package(PkgConfig ${SAIL_CODEC_JPEGXL_REQUIRED_OPTION})

if (NOT PkgConfig_FOUND)
    return()
endif()

pkg_check_modules(JPEGXL ${SAIL_CODEC_JPEGXL_REQUIRED_OPTION} libjxl libjxl_threads)

if (NOT JPEGXL_FOUND)
    return()
endif()

# Static libjxl pulls -lm through Libs.private. FindPkgConfig keeps the unresolved 'm'
# as a bare library name, and MSVC then fails to link a non-existent m.lib.
#
if (MSVC)
    list(REMOVE_ITEM JPEGXL_LINK_LIBRARIES m -lm)
endif()

# Static libjxl requires JXL_STATIC_DEFINE and friends to disable dllimport declarations.
# pkg-config reports them along with the other compile flags.
#
foreach (JPEGXL_CFLAG IN LISTS JPEGXL_CFLAGS_OTHER)
    if (JPEGXL_CFLAG MATCHES "^-D(.+)$")
        list(APPEND JPEGXL_COMPILE_DEFINITIONS ${CMAKE_MATCH_1})
    endif()
endforeach()

# For static builds, export dependencies
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "pkg_config,libjxl libjxl_threads")

# Static libjxl uses the Android logging API. pkg-config does not report it, and the codec
# itself needs it just as much as a client does: the exported entry below only reaches
# the link through sail-codecs, which exists when codecs are combined into an archive.
#
if (ANDROID)
    set(JPEGXL_SYSTEM_LIBRARIES log)

    list(JOIN JPEGXL_SYSTEM_LIBRARIES " " JPEGXL_SYSTEM_LIBRARIES_JOINED)
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "link_libraries,${JPEGXL_SYSTEM_LIBRARIES_JOINED}")
endif()

set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME jpegxl
            SOURCES helpers.h helpers.c jpegxl.c memory.h memory.c
            ICON jpegxl.png
            DEPENDENCY_COMPILE_DEFINITIONS ${JPEGXL_COMPILE_DEFINITIONS}
            DEPENDENCY_INCLUDE_DIRS ${JPEGXL_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${JPEGXL_LINK_LIBRARIES} ${JPEGXL_SYSTEM_LIBRARIES})
