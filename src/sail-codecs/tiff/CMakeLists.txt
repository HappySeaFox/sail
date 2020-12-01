# Common codec configuration
#
sail_codec(NAME tiff SOURCES helpers.c io.c tiff.c PKGCONFIG libtiff-4 SYSTEM_HEADERS tiff.h SYSTEM_LIBS tiff CMAKE ${CMAKE_CURRENT_LIST_DIR}/tiff.cmake)
