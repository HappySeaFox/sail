macro(sail_find_dependencies)
    find_library(AVIF_LIBRARY avif)
    find_path(AVIF_INCLUDE_DIRS avif/avif.h)

    if (NOT AVIF_LIBRARY OR NOT AVIF_INCLUDE_DIRS)
        return()
    endif()

    set(sail_avif_include_dirs ${AVIF_INCLUDE_DIRS})
    set(sail_avif_libs ${AVIF_LIBRARY})

    # This will add the following CMake rules to the CMake config for static builds so a client
    # application links against the required dependencies:
    #
    # find_dependency(LIBAVIF REQUIRED)
    # set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES avif)
    #
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_dependency,LIBAVIF,avif" PARENT_SCOPE)
endmacro()
