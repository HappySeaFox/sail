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
    target_include_directories(sail-sdl-viewer PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(sail-sdl-viewer PRIVATE ${SDL2_LIBRARIES})
endif()

# Depend on sail
#
target_link_libraries(sail-sdl-viewer PRIVATE sail)

# Enable ASAN if possible
#
sail_enable_asan(TARGET sail-sdl-viewer)
