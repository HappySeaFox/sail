# Common codec configuration
#
sail_codec(NAME jpeg SOURCES jpeg.c helpers.c io_dest.c io_src.c PKGCONFIG libjpeg CMAKE ${CMAKE_CURRENT_LIST_DIR}/jpeg.cmake)