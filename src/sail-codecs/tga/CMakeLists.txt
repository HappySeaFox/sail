# Common codec configuration
#
sail_codec(NAME tga SOURCES helpers.h helpers.c tga.c ICON tga.png)

# localtime_r
sail_enable_posix_c_source(TARGET ${SAIL_CODEC_TARGET} VERSION 200112L)
