# Require SDL2
#
find_package(SDL2 REQUIRED)

add_executable(sail-sdl-viewer sail-sdl-viewer.c)

# Add SDL2 dependencies
#
target_include_directories(sail-sdl-viewer PRIVATE "${SDL2_INCLUDE_DIRS}")
target_link_libraries(sail-sdl-viewer "${SDL2_LIBRARIES}")

# Depend on sail
#
target_link_libraries(sail-sdl-viewer sail)

# Enable ASAN
#
if (SAIL_ASAN)
    if (CMAKE_COMPILER_IS_GNUCC)
        target_compile_options(sail-sdl-viewer PRIVATE "${SAIL_ASAN}")
        target_link_libraries(sail-sdl-viewer "${SAIL_ASAN}")
    else()
        message("ASAN is not supported on this platform.")
    endif()
endif()
