macro(sail_find_dependencies)
    find_package(GIF)

    if (NOT GIF_FOUND)
        return()
    endif()

    set(sail_gif_include_dirs ${GIF_INCLUDE_DIRS})
    set(sail_gif_libs ${GIF_LIBRARIES})

    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "GIF,GIF::GIF" PARENT_SCOPE)
endmacro()
