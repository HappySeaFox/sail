# Changelog

SAIL is a fork of ksquirrel-libs, a set of C++ image codecs for KSquirrel.
See [http://ksquirrel.sourceforge.net](http://ksquirrel.sourceforge.net).

# SAIL (2020-now)

## 0.9.0

- first public release

# ksquirel-libs (until 0.8.0)

## 0.8.0

- added EPS, PSP
- bugfix in PNM decoder on P3 PNMs
- bugfix in CAMERA codec, when KSquirrel couldn't open raw fotos

## 0.7.3

- added DDS
- JPEG2000 decoder is much faster now

## 0.7.2

- SVG codec now uses rsvg-convert
- fixed segfault in GIF codec (in rare cases)
- added DICOM (requires medcon)
- added APNG (see http://wiki.mozilla.org/APNG_Specification, http://en.wikipedia.org/wiki/APNG)

## 0.7.1

- added XFIG (transfig package is required)
- added Degas PI3
- TTF codec is designed for freetype 2.2.1 (it should work at least in Debian Etch)
- CAMERA codec now uses dcraw v8.77 without modifications, which doesn't violate dcraw license
- bugfixes in scripts

## 0.7.0-pre2

- added IFF, MAC, PICT, PI1, XIM, UTAH, LEAF, NEO (via NetPBM package)
- small fix in configure.ac

## 0.7.0-pre1

- fixed segfault on GIF and some other libraries on some systems
- CAMERA codec now uses dcraw 8.61
- added settings support to CAMERA, SVG
- added DjVu (just single page via settings).
- added AutoCAD DXF (+settings)
- added GIMP XCF (+settings)
- small fixes in configure script
- configure output now more informatible

## 0.6.2

- some changes in API

## 0.6.1: bugfix release, minor changes

- some codecs are disabled as buggy (use --enable-devel to enable them)
- ksquirrel-libs now installs development library and header files. Now ksquirel-libs
  must be installed BEFORE KSquirrel.
- updated documentation

## 0.6.0

- SVG: added ".svgz" extension
- MNG: small fix in data types
- KOALA: small fix in bitdepth (bitdepth was always '0')

## 0.6.0-preview9 2005-11-09

- XCUR: fixed segfault
- PNM: small fix (on pgm images)
- SGI: small improvements
- CAMERA: new file extensions added
- added font support (ttf, pfa, pfb...)
- added MNG, JNG (reading)
- added PXR (reading)
- added JBIG (slow, reading))
- code cleanup
- no more static libraries. Now SVG, GIF, OPENEXR, WMF and JPEG2000 are optional

## 0.6.0-preview8 2005-10-10

- autoconf script code cleanup and bugfixes
- new options for configure script:
    `--disable-gif`
    `--disable-camera`
    `--disable-wmf`
    `--disable-svg`
    `--disable-openexr`
    `--disable-jpeg2000`
    `--disable-mng`
    `--disable-ttf`

## 0.6.0-preview7 2005-10-02

- code cleanup
- added numerous image filtering functions

## 0.6.0-preview6 2005-08-31

- some changes in the API
- added MTV Ray-Tracer (reading, writing)
- added AVS X (reading, writing)
- added PNM (writing)
- LIF: many bugfixes
- SVG: removed useless "-lfontconfig" dependency which could create compile problems on some systems (e.g. Slackware)
- PNG: fixed autoconf problems on some systems (e.g. Slackware)
- added new image filtering functions

## 0.6.0-preview5 2005-08-11

- added JPEG2000 (reading)
- added OpenEXR (reading)
- added Quake2 WAL texture (reading)
- added HalfLife model (reading)
- added KOALA (reading)
- added HDR (reading)
- added LIF (reading)
- added SCT (reading)
- CUT decoder is now not-alpha

## 0.6.0-preview4 2005-06-26

- cosmetic changes

## 0.6.0-preview3 2005-05-27

- some changes in the API makes prevew2 and prevew3 not compatible
- added SVG (reading, requires libxml2, freetype)
- TGA: small fix (in v0.7.1 flipping was ON by default, in v0.7.2 flipping is determined by image header)
- XPM: small fix in decoding mechanism
- new mime icons
- added image filtering C++ utils

## 0.6.0-preview2 2005-04-17

- changed installation path /usr/lib/squirrel -> /usr/lib/ksquirrel-libs
- fully migrated to C++ and classes
- added WMF (reading)
- added SUN Icon (reading)
- added WBMP (reading)
- added TIFF (writing)
- added RAW (photos from different cameras, e.g. CRW etc.) through dcraw
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

- GIF decoder is much better now (fixed issues with transparency)
- added X cursors
- added Adobe PSD (RGB,CMYK,Grayscale,Indexed)
- added FLI Animation
- small fixes in PNM and BMP codecs

## 0.5.0-preview4 2005-02-04

- migrated from Makefiles to autotools
- migrated all the rest codecs to the new C++ format
- added GIF (beta, including animated)
- interlaced PNG's
- fixed random crashing on some PNGs

## 0.5.0-preview2 2004-10-02

- migrated to C++ (without classes so far)
- have only three codecs migrated to C++: BPM, JPEG, PNG

# libSQlibs (until 0.2.8.1)

## 0.2.8.1 2004-05-19

- added lost in 0.2.8 license file
- added missing link rules for libjpeg, libpng, libtiff

## 0.2.8 2004-05-15

- added Makefiles
- added RAS

## 0.2.4 2004-04-27

- added LGPLv2 license
- added ICO, PCX, PIX, PNM, SGI, XBM, XPM, XWD

## 0.2.1 2004-04-06

- added TGA, TIFF

## 0.2.0 2004-03-31

- codecs are written in C
- codecs are compiled with shell scripts
- added BMP, JPG, PNG

## 0.0.0 2003-11-06

- first ideas about ksquirrel published at [linux.org.ru](https://www.linux.org.ru/forum/development/421774) (in Russian)
