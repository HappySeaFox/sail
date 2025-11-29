include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

find_path(RAW_INCLUDE_DIR libraw/libraw.h ${SAIL_CODEC_RAW_REQUIRED_OPTION})

if (SAIL_VCPKG)
    find_library(RAW_LIBRARY_RELEASE NAMES raw_r raw libraw_r libraw     PATHS "${RAW_INCLUDE_DIR}/../lib"       NO_DEFAULT_PATH)
    find_library(RAW_LIBRARY_DEBUG   NAMES raw_rd rawd libraw_rd librawd PATHS "${RAW_INCLUDE_DIR}/../debug/lib" NO_DEFAULT_PATH)
else()
    find_library(RAW_LIBRARY_RELEASE NAMES raw_r raw libraw_r libraw)
    find_library(RAW_LIBRARY_DEBUG   NAMES raw_rd rawd raw_r raw libraw_rd librawd libraw_r libraw)
endif()

select_library_configurations(RAW)

find_package_handle_standard_args(RAW
    REQUIRED_VARS
        RAW_INCLUDE_DIR
        RAW_LIBRARY)

if (NOT RAW_INCLUDE_DIR OR NOT RAW_LIBRARY)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_library(RAW_RELEASE_LIBRARY NAMES raw_r raw libraw_r libraw PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib" NO_DEFAULT_PATH)
# find_library(RAW_DEBUG_LIBRARY NAMES raw_rd rawd raw_r raw libraw_rd librawd libraw_r libraw PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib" NO_DEFAULT_PATH)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,raw_r raw libraw_r libraw,raw_rd rawd raw_r raw libraw_rd librawd libraw_r libraw" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME raw
            SOURCES datastream.h datastream.cpp helpers.h helpers.cpp raw.cpp
            ICON raw.png
            DEPENDENCY_INCLUDE_DIRS ${RAW_INCLUDE_DIR}
            DEPENDENCY_LIBS ${RAW_LIBRARY})

set_target_properties(sail-codec-raw PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO)

# getc_unlocked
sail_enable_posix_c_source(TARGET sail-codec-raw VERSION 200112L)
