find_package(PkgConfig ${SAIL_CODEC_RAW_REQUIRED_OPTION})

if (NOT PkgConfig_FOUND)
    return()
endif()

# Prefer the thread-safe flavor of LibRaw. vcpkg installs the non-thread-safe one
# into manual-link/ where it's not meant to be picked up automatically.
#
pkg_check_modules(RAW QUIET libraw_r)

if (RAW_FOUND)
    set(RAW_MODULE libraw_r)
else()
    pkg_check_modules(RAW ${SAIL_CODEC_RAW_REQUIRED_OPTION} libraw)
    set(RAW_MODULE libraw)
endif()

if (NOT RAW_FOUND)
    return()
endif()

# For static builds, export dependencies.
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "pkg_config,${RAW_MODULE}" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME raw
            SOURCES datastream.h datastream.cpp helpers.h helpers.cpp raw.cpp
            ICON raw.png
            DEPENDENCY_INCLUDE_DIRS ${RAW_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${RAW_LINK_LIBRARIES})

set_target_properties(sail-codec-raw PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO)

# getc_unlocked
sail_enable_posix_c_source(TARGET sail-codec-raw VERSION 200112L)
