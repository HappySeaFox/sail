# ChangeLog

SAIL is rebranded ksquirrel-libs rewritten in C, improved and with high-level APIs. Ksquirrel-libs was a set of C++ image codecs
for the KSquirrel image viewer. See [http://ksquirrel.sourceforge.net](http://ksquirrel.sourceforge.net).

See [README](README.md) for more.

# SAIL (since 0.9.0)

## 0.9.0 XXXX-XX-XX

- first public release
- implemented a rich C client API to load and save images
- implemented a rich C++ client API to load and save images
- codecs interfaces are now hidden. Always use the client APIs to load or save images

# ksquirrel-libs (until 0.8.0)

## 0.8.0 2007-12-08

- added EPS (reading)
- added PSP (reading)
- 59 codecs in total
- bugfix in PNM decoder on P3 PNMs
- bugfix in CAMERA codec, when KSquirrel couldn't open raw fotos

## 0.7.5 2007-11-14

- CAMERA codec now uses dcraw v8.79
- update emails to ksquirrel.iv@gmail.com

## 0.7.4 2007-11-05

- no changes

## 0.7.3 2007-11-04

- added DDS (reading)
- 57 codecs in total
- JPEG2000 decoder is much faster now
- check for errors when external converter programs are executed

## 0.7.2 2007-10-17

- added DICOM (reading, requires medcon)
- added APNG (reading, see [http://en.wikipedia.org/wiki/APNG](http://en.wikipedia.org/wiki/APNG))
- 56 codecs in total
- fixed rare crashes with some GIFs
- added LJPEG missing data
- added TIFF settings UI
- SVG codec now uses rsvg-convert
- code cleanup

## 0.7.1-try3 2007-09-21

- fixed random crashes with GIFs

## 0.7.1-try2 2007-09-14

- added LJPEG (reading)
- 54 codecs in total
- configure script now checks for libungif

## 0.7.1 2007-08-30

- added XFIG (reading, transfig package is required)
- added Degas PI3 (reading)
- 53 codecs in total
- added mime types
- TTF codec is designed for freetype 2.2.1 (it should work fine at least in Debian Etch)
- CAMERA codec now uses dcraw v8.77 without modifications, so it doesn't violate dcraw license anymore
- bugfixes in scripts

## 0.7.0 2007-05-13

- updated Qt examples

## 0.7.0-preview2 2007-04-09

- added LEAF, NEO, PI1, PICT, XIM via NetPBM package
- 51 codecs in total
- small fixes in configure.ac

## 0.7.0-preview1 2007-03-09

- added DjVu (reading, just single page via settings)
- added AutoCAD DXF (reading, +settings)
- added GIMP XCF (reading, +settings)
- 46 codecs in total
- fixed random segfaults on GIFs and some other codecs
- CAMERA codec now uses dcraw 8.61
- added settings support to CAMERA, SVG
- small fixes in configure script
- configure output is more verbose now

## 0.6.2 2006-01-02

- API changes

## 0.6.1 2005-12-21

- MAC, MSP, IFF, and UTAH codecs are disabled as buggy (use --enable-devel to enable them)
- ksquirrel-libs now installs a development library and header files
- ksquirrel-libs now must be installed before KSquirrel
- updated documentation

## 0.6.0 2005-12-10

- SVG: added ".svgz" extension
- MNG: small fix in data types
- KOALA: small fix in bitdepth (bitdepth was always '0')
- security fixes

## 0.6.0-preview9 2005-11-09

- added TTF fonts support (ttf, pfa, pfb...)
- added MNG, JNG (reading)
- added JBIG (slow, reading))
- 43 codecs in total
- XCUR: fixed segfault
- PNM: small fix on pgm images
- SGI: small improvements
- CAMERA: new file extensions added
- code cleanup
- no more static libraries. Now SVG, GIF, OPENEXR, WMF, and JPEG2000 are optional

## 0.6.0-preview8 2005-10-10

- autoconf script cleanup and bugfixes
- new options in configure script:
    `--disable-gif`
    `--disable-camera`
    `--disable-wmf`
    `--disable-svg`
    `--disable-openexr`
    `--disable-jpeg2000`
    `--disable-mng`
    `--disable-ttf`

## 0.6.0-preview7 2005-10-02

- added PXR (reading)
- 40 codecs in total
- code cleanup
- added numerous image filtering functions

## 0.6.0-preview6 2005-08-31

- added MTV Ray-Tracer (reading, writing)
- added AVS X (reading, writing)
- added MAC (reading)
- added MSP (reading)
- 39 codecs in total
- added writing features to PNM
- some changes in the API
- LIF: many bugfixes
- SVG: removed useless "-lfontconfig" dependency which could lead to compile errors on some systems (e.g. Slackware)
- PNG: fixed autoconf errors on some systems (e.g. Slackware)
- added new image filtering functions

## 0.6.0-preview5 2005-08-11

- added JPEG2000 (reading)
- added OpenEXR (reading)
- added WAL Quake2 texture (reading)
- added MDL HalfLife model (reading)
- added KOALA (reading)
- added HDR (reading)
- added LIF (reading)
- added SCT (reading)
- 35 codecs in total
- CUT decoder is now not-alpha

## 0.6.0-preview4 2005-06-26

- cosmetic changes

## 0.6.0-preview3 2005-05-27

- added SVG (reading, requires libxml2, freetype)
- 27 codecs in total
- some changes in the API makes prevew2 and prevew3 not compatible
- TGA: small fix (in v0.7.1 flipping was ON by default, in v0.7.2 flipping is determined by image header)
- XPM: small fix in decoding mechanism
- new mime icons
- added image filtering C++ utils

## 0.6.0-preview2 2005-04-17

- added CUT (reading)
- added IFF (reading)
- added WMF (reading)
- added SUN Icon (reading)
- added WBMP (reading)
- added TIFF (writing)
- added RAWRGB (reading, writing)
- added RAW (photos from different cameras, e.g. CRW etc.) through dcraw
- 26 codecs in total
- changed installation path /usr/lib/squirrel -> /usr/lib/ksquirrel-libs
- fully migrated to C++ and classes
- added some examples (Qt, Qt+OpenGL)

## 0.6.0-preview1 2005-02-28

- added interface for write functions
- added write features for PNG, JPEG, BMP
- PNG: fixed issues with interlaced images, fixed memory leaks
- XPM: fixed issues with multiline comments
- ICO: added support of bit depth 24 and 32
- PSD: fixed issues with RGB images which have 3 channels instead of 4
- GIF: added comments support (comment extensions)
- PNM: fixed issues with Windows-like line breaks (\r\n)

## 0.5.0 2005-02-20

- added X cursors
- added Adobe PSD (RGB,CMYK,Grayscale,Indexed)
- added FLI Animation
- 18 codecs in total
- GIF decoder is much better now (fixed issues with transparency)
- small fixes in PNM and BMP codecs

## 0.5.0-preview4 2005-02-04

- added GIF (beta, including animated)
- 15 codecs in total
- migrated from Makefiles to autotools
- migrated all the rest codecs to the new C++ format
- interlaced PNG's
- fixed random crashing on some PNGs

## 0.5.0-preview2 2004-10-02

- migrated to C++ (without classes so far)
- have only three codecs migrated to C++: BPM, JPEG, PNG

# libSQlibs (until 0.2.8.1)

## 0.2.8.1 2004-05-19

- added back the license file lost in 0.2.8
- added missing link rules for libjpeg, libpng, libtiff

## 0.2.8 2004-05-15

- added Makefiles
- added RAS
- 14 codecs in total

## 0.2.4 2004-04-27

- added LGPLv2 license
- added ICO, PCX, PIX, PNM, SGI, XBM, XPM, XWD
- 13 codecs in total

## 0.2.1 2004-04-06

- added TGA, TIFF
- 5 codecs in total

## 0.2.0 2004-03-31

- added BMP, JPG, PNG
- 3 codecs in total
- codecs are written in C
- codecs are compiled with shell scripts

## 0.0.0 2003-11-06

- started active development
- first ideas about KSquirrel were published at [linux.org.ru](https://www.linux.org.ru/forum/development/421774) (in Russian)
