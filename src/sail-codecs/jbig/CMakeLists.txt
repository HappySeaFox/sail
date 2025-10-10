# Try to find JBIG library
#
find_library(JBIG_LIBRARY NAMES jbig)
find_path(JBIG_INCLUDE_DIR NAMES jbig.h)

if (NOT JBIG_LIBRARY OR NOT JBIG_INCLUDE_DIR)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,jbig,jbigd jbig" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME jbig
            SOURCES helpers.h helpers.c jbig.c
            ICON jbig.png
            DEPENDENCY_INCLUDE_DIRS ${JBIG_INCLUDE_DIR}
            DEPENDENCY_LIBS ${JBIG_LIBRARY})

