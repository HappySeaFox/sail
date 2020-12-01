# Common codec configuration
#
sail_codec(NAME png SOURCES helpers.c io.c png.c PKGCONFIG libpng16 CMAKE ${CMAKE_CURRENT_LIST_DIR}/png.cmake)
