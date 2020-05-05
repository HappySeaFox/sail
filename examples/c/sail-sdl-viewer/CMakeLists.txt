add_executable(sail-sdl-viewer sail-sdl-viewer.c)

# Add SDL2 dependencies
#
target_include_directories(sail-sdl-viewer PRIVATE "${SDL2_INCLUDE_DIRS}")
target_link_libraries(sail-sdl-viewer "${SDL2_LIBRARIES}")

# Depend on sail
#
target_link_libraries(sail-sdl-viewer sail)

# Enable ASAN if possible
#
sail_enable_asan(TARGET sail-sdl-viewer)
