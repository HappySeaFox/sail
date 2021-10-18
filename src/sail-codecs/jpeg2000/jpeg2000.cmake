macro(sail_find_dependencies)
    find_library(JPEG2000_RELEASE_LIBRARY NAMES jasper)
    find_library(JPEG2000_DEBUG_LIBRARY NAMES jasperd jasper)
    find_path(JPEG2000_INCLUDE_DIRS jasper/jasper.h)

    if ((NOT JPEG2000_RELEASE_LIBRARY AND NOT JPEG2000_DEBUG_LIBRARY) OR NOT JPEG2000_INCLUDE_DIRS)
        return()
    endif()

    set(JPEG2000_LIBRARY optimized ${JPEG2000_RELEASE_LIBRARY} debug ${JPEG2000_DEBUG_LIBRARY})

    set(sail_jpeg2000_include_dirs ${JPEG2000_INCLUDE_DIRS})
    set(sail_jpeg2000_libs ${JPEG2000_LIBRARY})

    # This will add the following CMake rules to the CMake config for static builds so a client
    # application links against the required dependencies:
    #
    # find_library(jasper_RELEASE_LIBRARY NAMES jasper)
    # find_library(jasper_DEBUG_LIBRARY NAMES jasperd jasper)
    # set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES $<$<CONFIG:Release>:${jasper_RELEASE_LIBRARY}> $<$<CONFIG:Debug>:${jasper_DEBUG_LIBRARY}>)
    #
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_library,jasper,jasperd" PARENT_SCOPE)
endmacro()
