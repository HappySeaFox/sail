include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

find_path(JBIG_INCLUDE_DIR jbig.h)

# Find JBIG library
#
if (SAIL_VCPKG)
    find_library(JBIG_LIBRARY_RELEASE NAMES jbig libjbig PATHS "${JBIG_INCLUDE_DIR}/../lib"       NO_DEFAULT_PATH)
    find_library(JBIG_LIBRARY_DEBUG   NAMES jbig libjbig PATHS "${JBIG_INCLUDE_DIR}/../debug/lib" NO_DEFAULT_PATH)
else()
    find_library(JBIG_LIBRARY_RELEASE NAMES jbig libjbig)
    find_library(JBIG_LIBRARY_DEBUG   NAMES jbig libjbig)
endif()

select_library_configurations(JBIG)

find_package_handle_standard_args(JBIG
    REQUIRED_VARS
        JBIG_INCLUDE_DIR
        JBIG_LIBRARY)

if (NOT JBIG_INCLUDE_DIR OR NOT JBIG_LIBRARY)
    if (SAIL_CODEC_JBIG_REQUIRED_OPTION STREQUAL "REQUIRED")
        message(FATAL_ERROR "JBIG: Missing dependencies")
    else()
        return()
    endif()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_library(jbig_RELEASE_LIBRARY NAMES jbig libjbig)
# find_library(jbig_DEBUG_LIBRARY NAMES jbig libjbig)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES $<$<CONFIG:Release>:${jbig_RELEASE_LIBRARY}> $<$<CONFIG:Debug>:${jbig_DEBUG_LIBRARY}>)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,jbig libjbig,jbig libjbig" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME jbig
            SOURCES helpers.h helpers.c jbig.c
            ICON jbig.png
            DEPENDENCY_INCLUDE_DIRS ${JBIG_INCLUDE_DIR}
            DEPENDENCY_LIBS ${JBIG_LIBRARY})
