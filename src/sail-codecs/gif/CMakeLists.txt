# Common codec configuration
#
sail_codec(NAME gif SOURCES helpers.c io.c gif.c PKGCONFIG giflib SYSTEM_HEADERS gif_lib.h SYSTEM_LIBS gif)
