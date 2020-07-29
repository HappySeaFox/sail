# Common plugin configuration
#
sail_plugin(TARGET tiff SOURCES helpers.c io.c tiff.c PKGCONFIG libtiff-4 libjpeg zlib)
