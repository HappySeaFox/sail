<h1 align="center">Squirrel Abstract Image Library</h1>
<h3 align="center">The missing fast and easy-to-use image decoding library for humans (not for machines).</h3>
<p align="center">
  <a href="https://app.travis-ci.com/HappySeaFox/sail">
    <img alt="Travis Build Status" src="https://img.shields.io/travis/com/HappySeaFox/sail"/>
  </a>
  <a href="https://lgtm.com/projects/g/HappySeaFox/sail/context:cpp">
    <img alt="Language grade: C/C++" src="https://img.shields.io/lgtm/grade/cpp/g/HappySeaFox/sail.svg?logo=lgtm&logoWidth=18"/>
  </a>
  <a href="https://github.com/HappySeaFox/sail/releases">
    <img alt="Latest release" src="https://img.shields.io/github/v/release/HappySeaFox/sail?include_prereleases"/>
  </a>
</p>
<p align="center">
  <a href="#target-audience">Target Audience</a> •
  <a href="#features-overview">Features</a> •
  <a href="#supported-image-formats">Image Formats</a> •
  <a href="#apis-overview">Getting Started</a> •
  <a href="FAQ.md">FAQ</a>
</p>

#

SAIL is a format-agnostic cross-platform image decoding library providing rich APIs, from one-liners
to complex use cases with custom I/O sources. It enables a client to load and save static,
animated, multi-paged images along with their meta data and ICC profiles. :sailboat:

<p align="center">
  <a href=".github/qt-demo.gif"><img src=".github/qt-demo.gif?raw=true" alt="GIF Demo Screenshot"/></a>
</p>

## Target audience

- Image viewers
- Game developers
- Anyone who needs to load or save images in different image formats and who needs
  a clean and comprehensive API for that

## Features overview

- [x] Easy-to-use thread-safe C and C++ interfaces
- [x] Versatile APIs: `junior`, `advanced`, `deep diver`, and `technical diver`
- [x] Input/output: files, memory, custom I/O streams
- [x] Load by file suffixes, paths, and [magic numbers](https://en.wikipedia.org/wiki/File_format#Magic_number)
- [x] Save pixels as close as possible to the source
- [x] Codec-specific tuning options like <a href="https://en.wikipedia.org/wiki/Portable_Network_Graphics#Filtering">PNG filters</a>. See [FORMATS](FORMATS.md)
- [x] Meta data support: text comments, EXIF, ICC profiles
- [x] Access to the image properties w/o decoding pixels (probing)
- [x] Access to the source image properties
- [x] Adding or updating image codecs with ease demonstrated by Intel \[[*](#intel)\]
- [x] The best MIME icons in the computer industry :smile:

<a id="intel"></a>

\* One day Intel demonstrated the advantages of their [IPP](https://wikipedia.org/wiki/Integrated_Performance_Primitives) technology in speeding up decoding
[JPEG](https://web.archive.org/web/20091009223918/http://software.intel.com/en-us/articles/intel-integrated-performance-primitives-intel-ipp-for-linux-optimizing-jpeg-coding-in-the-ksquirrel-application-with-intel-ipp)
and
[JPEG2000](https://web.archive.org/web/20091009224048/http://software.intel.com/en-us/articles/performance-tools-for-software-developers-application-notes-intel-ipp-jpeg2000-and-jasper-in-ksquirrel)
images with the help of [ksquirrel-libs](FAQ.md#how-old-is-sail), the predecessor of SAIL.

## Features NOT provided

- [ ] Image editing capabilities (filtering, distortion, scaling, etc.)
- [ ] Color space conversion functions
- [ ] Color management functions (applying ICC profiles etc.)
- [ ] EXIF rotation

## Supported image formats

| N  | Image format                                                        | Operations    | Dependencies      |
| -- | --------------------------------------------------------------------| ------------- | ----------------- |
| 1  | [APNG](https://wikipedia.org/wiki/APNG)                             | R             | libpng+APNG patch |
| 2  | [AVIF](https://wikipedia.org/wiki/AV1#AV1_Image_File_Format_(AVIF)) | R             | libavif           |
| 3  | [BMP](https://wikipedia.org/wiki/BMP_file_format)                   | R             |                   |
| 4  | [GIF](https://wikipedia.org/wiki/GIF)                               | R             | giflib            |
| .. | ...                                                                 |               |                   |
| 6  | [JPEG](https://wikipedia.org/wiki/JPEG)                             | RW            | libjpeg-turbo     |
| 7  | [JPEG2000](https://wikipedia.org/wiki/JPEG_2000)                    | R             | jasper            |
| 8  | [PCX](https://wikipedia.org/wiki/PCX)                               | R             |                   |
| 9  | [PNG](https://wikipedia.org/wiki/Portable_Network_Graphics)         | RW            | libpng            |
| 10 | [QOI](http://qoiformat.org)                                         | RW            |                   |
| 11 | [SVG](https://wikipedia.org/wiki/Scalable_Vector_Graphics)          | R             | resvg             |
| 12 | [TGA](https://wikipedia.org/wiki/Truevision_TGA)                    | R             |                   |
| 13 | [TIFF](https://wikipedia.org/wiki/TIFF)                             | RW            | libtiff           |
| .. | ...                                                                 |               |                   |
| 15 | [WEBP](https://wikipedia.org/wiki/WebP)                             | R             | libwebp           |
| .. | ...                                                                 |               |                   |

See the full list [here](FORMATS.md). Work to add more image formats is ongoing.

## Benchmarks

<p align="center">
  <a href="BENCHMARKS.md"><img src=".github/benchmarks/JPEG-YCbCr-6000x4016.png?raw=true" alt="Benchmark" width="500px"/></a>
</p>

Time to load and output default pixels (without explicit conversion) was measured. See [BENCHMARKS](BENCHMARKS.md).

## Preferred installation method

- Windows: `conan`, `vcpkg`
- macOS: `conan`, `brew`
- Linux: native packages if available, `conan`, `vcpkg`

See [BUILDING](BUILDING.md).

## APIs overview

SAIL provides four levels of APIs, depending on your needs. Let's have a quick look at the `junior` level.

#### C:
```C
struct sail_image *image;

SAIL_TRY(sail_load_image_from_file(path, &image));

/*
 * Handle the image pixels here.
 * Use image->width, image->height, image->bytes_per_line,
 * image->pixel_format, and image->pixels for that.
 *
 * In particular, you can convert it to a different pixel format with functions
 * from libsail-manip. With sail_convert_image(), for example.
 */

sail_destroy_image(image);
```

#### C++:
```C++
sail::image image(path);

// Handle the image and its pixels here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.pixels() for that.
//
// In particular, you can convert it to a different pixel format with image::convert().
```

It's pretty easy, isn't it? :smile: See also [FAQ](FAQ.md).

## Programming languages

**Programming language:** C11<br/>
**Bindings:** C++11

## Competitors

- [FreeImage](https://freeimage.sourceforge.io)
- [DevIL](http://openil.sourceforge.net)
- [SDL_Image](https://www.libsdl.org/projects/SDL_image)
- [stb_image](https://github.com/nothings/stb)
- [Boost.GIL](https://www.boost.org/doc/libs/1_68_0/libs/gil/doc/html/index.html)
- [gdk-pixbuf](https://developer.gnome.org/gdk-pixbuf)
- [imlib2](https://docs.enlightenment.org/api/imlib2/html)
- [CImg](https://github.com/dtschump/CImg)
- [WIC (Windows only)](https://docs.microsoft.com/en-us/windows/win32/wic/-wic-about-windows-imaging-codec)

## Differences from other image decoding libraries

- Easily extensible with new image format plugins
- Easy-to-use API providing expected business entities - images, palettes, pixels etc.
- Access to source pixel data (supported by the most codecs)
- Access to the image properties w/o decoding pixel data (probing)

## Development status

SAIL is ready for every day use. However, it's still under heavy development. The API can be changed at any time
breaking binary and source compatibility. Consider opening a GitHub [issue](https://github.com/smoked-herring/sail/issues)
if you have any feature requests or issue reports. Your help (pull requests etc.) is highly welcomed.

## Have questions or issues?

Opening a GitHub [issue](https://github.com/smoked-herring/sail/issues) is the preferred way
of communicating and solving problems.

See [FAQ](FAQ.md) for more.

## Architecture overview

SAIL is written in pure C11 w/o using any third-party libraries (except for codecs). It also provides
bindings to C++.

### SAIL codecs

SAIL codecs is the deepest level. This is a set of standalone, dynamically loaded codecs (SO on Linux
and DLL on Windows). They implement actual decoding and encoding capabilities. End-users never work with
codecs directly. They always use abstract, high-level APIs in `libsail` for that.

Every codec is accompanied with a so called codec info (description) file which is just a plain text file.
It describes what the codec can actually do: what pixel formats it can load and output, what compression types
it supports, and more.

By default, SAIL loads codecs on demand. To preload them, use `sail_init_with_flags(SAIL_FLAG_PRELOAD_CODECS)`.

### libsail-common

libsail-common holds common data types (images, pixel formats, I/O abstractions etc.) and a small set
of functions shared between SAIL codecs and the high-level APIs in `libsail`.

### libsail

libsail is a feature-rich, high-level API. It provides comprehensive and lightweight interfaces to decode
and encode images. End-users implementing C applications always work with libsail.

### libsail-manip

libsail-manip is a collection of image manipulation functions. For example, conversion functions from one pixel
format to another.

### libsail-c++

libsail-c++ is a C++ binding to libsail. End-users implementing C++ applications may choose
between libsail and libsail-c++. Using libsail-c++ is always recommended, as it's much more simple
to use in C++ applications.

## Building

See [BUILDING](BUILDING.md).

## Philosophy

Philosophy of SAIL is modularization and simplicity.

Image codecs are architectured to be standalone dynamically loaded files. Any future hypothetical improvements
will be implemented as separate client libraries. So a user is always able to choose what to use (i.e. to link against)
and what not to use.

## Support

If you like the project, please consider starring the repository.

## Author

Dmitry Baryshev

## License

Released under the MIT license.

```
Copyright (c) 2020-2022 Dmitry Baryshev

The MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
