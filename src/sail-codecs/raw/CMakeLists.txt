find_path(RAW_INCLUDE_DIR libraw/libraw.h ${SAIL_CODEC_RAW_REQUIRED_OPTION})
find_library(RAW_LIBRARY NAMES raw_r raw libraw_r libraw ${SAIL_CODEC_RAW_REQUIRED_OPTION})

if (NOT RAW_INCLUDE_DIR OR NOT RAW_LIBRARY)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_library(RAW_RELEASE_LIBRARY NAMES raw_r raw libraw_r libraw PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib" NO_DEFAULT_PATH)
# find_library(RAW_DEBUG_LIBRARY NAMES raw_r raw libraw_r libraw PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib" NO_DEFAULT_PATH)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,raw_r raw libraw_r libraw,raw_r raw libraw_r libraw" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME raw
            SOURCES helpers.h helpers.cpp datastream.h datastream.cpp raw.cpp
            ICON raw.png
            DEPENDENCY_INCLUDE_DIRS ${RAW_INCLUDE_DIR}
            DEPENDENCY_LIBS ${RAW_LIBRARY})

set_target_properties(sail-codec-raw PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO)
