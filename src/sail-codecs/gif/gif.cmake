macro(sail_find_dependencies)
    find_package(GIF)

    if (NOT GIF_FOUND)
        return()
    endif()

    set(sail_gif_include_dirs ${GIF_INCLUDE_DIRS})
    set(sail_gif_libs ${GIF_LIBRARIES})

    # This will add the following CMake rules to the CMake config for static builds so a client
    # application links against the required dependencies:
    #
    # find_dependency(GIF REQUIRED)
    # set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES GIF::GIF)
    #
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "GIF,GIF::GIF" PARENT_SCOPE)
endmacro()
