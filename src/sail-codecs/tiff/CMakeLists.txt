# Common codec configuration
#
sail_codec(NAME tiff SOURCES helpers.h helpers.c io.h io.c tiff.c CMAKE ${CMAKE_CURRENT_LIST_DIR}/tiff.cmake)
