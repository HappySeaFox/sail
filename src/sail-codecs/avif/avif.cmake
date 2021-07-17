macro(sail_find_dependencies)
    find_library(AVIF_LIBRARY avif)
    find_path(AVIF_INCLUDE_DIRS avif/avif.h)

    if (NOT AVIF_LIBRARY OR NOT AVIF_INCLUDE_DIRS)
        return()
    endif()

    set(sail_avif_include_dirs ${AVIF_INCLUDE_DIRS})
    set(sail_avif_libs ${AVIF_LIBRARY})

    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "AVIF,${AVIF_LIBRARY}" PARENT_SCOPE)
endmacro()
