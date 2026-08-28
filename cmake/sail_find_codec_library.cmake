# Intended to be included by codecs. Finds a single optional dependency library
# the same way across codecs: it respects vcpkg release/debug layouts and merges
# the found configurations into ${PREFIX}_LIBRARY with select_library_configurations.
#
# Arguments:
#   PREFIX            - variable name prefix. The result is exported as ${PREFIX}_LIBRARY
#   NAMES_RELEASE     - library names to search for in release builds
#   NAMES_DEBUG       - library names to search for in debug builds (defaults to NAMES_RELEASE)
#   VCPKG_INCLUDE_DIR - a found include dir used to derive vcpkg lib/ and debug/lib paths
#   OUTPUT_LIBS       - optional list variable the found library is appended to
#
macro(sail_find_codec_library)
    cmake_parse_arguments(SAIL_FCL "" "PREFIX;VCPKG_INCLUDE_DIR;OUTPUT_LIBS" "NAMES_RELEASE;NAMES_DEBUG" ${ARGN})

    include(SelectLibraryConfigurations)

    if (NOT SAIL_FCL_NAMES_DEBUG)
        set(SAIL_FCL_NAMES_DEBUG ${SAIL_FCL_NAMES_RELEASE})
    endif()

    if (SAIL_VCPKG AND SAIL_FCL_VCPKG_INCLUDE_DIR)
        find_library("${SAIL_FCL_PREFIX}_LIBRARY_RELEASE" NAMES ${SAIL_FCL_NAMES_RELEASE} PATHS "${SAIL_FCL_VCPKG_INCLUDE_DIR}/../lib"       NO_DEFAULT_PATH)
        find_library("${SAIL_FCL_PREFIX}_LIBRARY_DEBUG"   NAMES ${SAIL_FCL_NAMES_DEBUG}   PATHS "${SAIL_FCL_VCPKG_INCLUDE_DIR}/../debug/lib" NO_DEFAULT_PATH)
    else()
        find_library("${SAIL_FCL_PREFIX}_LIBRARY_RELEASE" NAMES ${SAIL_FCL_NAMES_RELEASE})
        find_library("${SAIL_FCL_PREFIX}_LIBRARY_DEBUG"   NAMES ${SAIL_FCL_NAMES_DEBUG})
    endif()

    select_library_configurations(${SAIL_FCL_PREFIX})

    if (${SAIL_FCL_PREFIX}_LIBRARY AND SAIL_FCL_OUTPUT_LIBS)
        list(APPEND ${SAIL_FCL_OUTPUT_LIBS} ${${SAIL_FCL_PREFIX}_LIBRARY})
    endif()
endmacro()
