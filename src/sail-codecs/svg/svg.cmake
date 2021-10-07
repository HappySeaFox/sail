macro(sail_find_dependencies)
    find_library(SVG_LIBRARY resvg)
    find_path(SVG_INCLUDE_DIRS resvg.h)

    if (NOT SVG_LIBRARY OR NOT SVG_INCLUDE_DIRS)
        return()
    endif()

    set(sail_svg_include_dirs ${SVG_INCLUDE_DIRS})
    set(sail_svg_libs ${SVG_LIBRARY})

    # This will add the following CMake rules to the CMake config for static builds so a client
    # application links against the required dependencies:
    #
    # find_dependency(SVG_LIBRARY resvg REQUIRED)
    # set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${SVG_LIBRARY})
    #
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,resvg," PARENT_SCOPE)
endmacro()
