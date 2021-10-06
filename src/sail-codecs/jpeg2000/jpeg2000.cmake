macro(sail_find_dependencies)
    find_library(JPEG2000_LIBRARY jasper)
    find_path(JPEG2000_INCLUDE_DIRS jasper/jasper.h)

    if (NOT JPEG2000_LIBRARY OR NOT JPEG2000_INCLUDE_DIRS)
        return()
    endif()

    set(sail_jpeg2000_include_dirs ${JPEG2000_INCLUDE_DIRS})
    set(sail_jpeg2000_libs ${JPEG2000_LIBRARY})

    # This will add the following CMake rules to the CMake config for static builds so a client
    # application links against the required dependencies:
    #
    # find_dependency(JPEG2000 REQUIRED)
    # set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${JPEG2000_LIBRARY})
    #
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "JASPER,${JPEG2000_LIBRARY}" PARENT_SCOPE)
endmacro()
