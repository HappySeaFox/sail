macro(sail_find_dependencies)
    find_library(WEBP_LIBRARY webp)
    find_path(WEBP_INCLUDE_DIRS webp/decode.h)

    if (NOT WEBP_LIBRARY OR NOT WEBP_INCLUDE_DIRS)
        return()
    endif()

    set(sail_webp_include_dirs ${WEBP_INCLUDE_DIRS})
    set(sail_webp_libs ${WEBP_LIBRARY})

    # This will add the following CMake rules to the CMake config for static builds so a client
    # application links against the required dependencies:
    #
    # find_dependency(WebP REQUIRED)
    # set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${WEBP_LIBRARY})
    #
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "WebP,${WEBP_LIBRARY}" PARENT_SCOPE)
endmacro()
