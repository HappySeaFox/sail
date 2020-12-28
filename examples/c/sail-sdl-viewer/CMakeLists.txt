add_executable(sail-sdl-viewer sail-sdl-viewer.c)

# Add SDL2 dependencies
#
if (SAIL_VCPKG)
    if (UNIX)
        target_link_libraries(sail-sdl-viewer PRIVATE SDL2::SDL2-static SDL2::SDL2main)
    else()
        target_link_libraries(sail-sdl-viewer PRIVATE SDL2::SDL2 SDL2::SDL2main)
    endif()
else()
    # On OSX and brew, SDL2_INCLUDE_DIRS points to .../SDL2 which makes impossible
    # inclusion of SDL2/SDL.h. For some reason, /usr/local/include is not included
    # into the search paths, so we do it explicitly.
    #
    if (APPLE)
        find_path(SAIL_SDL2_INCLUDE_DIR NAMES SDL2/SDL.h)
        target_include_directories(sail-sdl-viewer PRIVATE ${SAIL_SDL2_INCLUDE_DIR})
    endif()

    target_include_directories(sail-sdl-viewer PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(sail-sdl-viewer PRIVATE ${SDL2_LIBRARIES})
endif()

# Depend on sail
#
target_link_libraries(sail-sdl-viewer PRIVATE sail)

# Enable ASAN if possible
#
sail_enable_asan(TARGET sail-sdl-viewer)
