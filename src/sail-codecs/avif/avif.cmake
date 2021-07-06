macro(sail_find_dependencies)
    find_library(AVIF_LIBRARY avif)

    if (NOT AVIF_LIBRARY)
        return()
    endif()

    set(sail_avif_include_dirs ${AVIF_INCLUDE_DIRS})
    set(sail_avif_libs ${AVIF_LIBRARY})

    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "AVIF,${AVIF_LIBRARY}" PARENT_SCOPE)
endmacro()
