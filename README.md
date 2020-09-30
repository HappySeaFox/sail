# SAIL: Squirrel Abstract Image Library

SAIL is a fast and lightweight cross-platform image decoding and encoding library providing rich APIs,
from one-liners to complex use cases with custom I/O sources. It enables a client to read and write static,
animated, multi-paged images along with their meta information and ICC profiles. :sailboat:


**Latest release:** unreleased <br/>
**Latest pre-release:** [v0.9.0-pre8](https://github.com/smoked-herring/sail/releases/tag/v0.9.0-pre8)<br/>
**Programming language:** C<br/>
**Bindings:** C++<br/>
**License:** MIT

See [FAQ](FAQ.md) for more.

Author: Dmitry Baryshev

![Screenshot](.github/qt-demo.png?raw=true)

## Target audience

- Image viewers
- Game developers
- Anyone who needs to load or save images in different image formats and who requires
  a lightweight and comprehensive API for that

## Features overview

- Easy-to-use C and C++ interfaces
- Four levels of APIs, depending on your needs: `junior`, `advanced`, `deep diver`, and `technical diver`.
  See [EXAMPLES](EXAMPLES.md) for more
- Reading images from file, memory, and custom I/O streams
- Writing images to file, memory, and custom I/O streams
- Detecting image types by file path, file extension, and [magic numbers](https://en.wikipedia.org/wiki/File_format#Magic_number)
- Reading operations output `BPP32-RGBA` pixels by default
- Most image format codecs are able to output the `SOURCE` pixel format for those who want
  to kick the hell out of images manually. For example, one may want to work with `CMYK` pixels in a print image
- Read and write ICC profiles
- Read and write meta information like JPEG comments
- Image formats are supported through dynamically loaded SAIL codecs
- Qt, SDL, and pure C examples
- The best MIME icons in the computer industry

## Features NOT provided

- Image editing capabilities (filtering, distortion, scaling, etc.)
- Color space conversion functions
- Color management functions (applying ICC profiles etc.)
- EXIF rotation

## Supported image formats

| N  | Image format                                                                            | Operations    | Dependencies      |
| -- | --------------------------------------------------------------------------------------- | ------------- | ----------------- |
| 1  | [APNG (Animated Portable Network Graphics)](https://wikipedia.org/wiki/APNG)            | R             | libpng+APNG patch |
| 2  | [JPEG (Joint Photographic Experts Group)](https://wikipedia.org/wiki/JPEG)              | RW            | libjpeg-turbo     |
| 3  | [PNG (Portable Network Graphics)](https://wikipedia.org/wiki/Portable_Network_Graphics) | RW            | libpng            |
| 4  | [TIFF (Tagged Image File Format)](https://wikipedia.org/wiki/TIFF)                      | RW            | libtiff           |

Work to add more image formats is ongoing.

## Supported platforms

Currently, SAIL supports the following platforms:

- Windows
- macOS
- Linux

## Competitors

- [FreeImage](https://freeimage.sourceforge.io)
- [DevIL](http://openil.sourceforge.net)
- [SDL_Image](https://www.libsdl.org/projects/SDL_image)
- [stb_image](https://github.com/nothings/stb)
- [Boost.GIL](https://www.boost.org/doc/libs/1_68_0/libs/gil/doc/html/index.html)
- [gdk-pixbuf](https://developer.gnome.org/gdk-pixbuf)
- [imlib2](https://docs.enlightenment.org/api/imlib2/html)
- [WIC (Windows only)](https://docs.microsoft.com/en-us/windows/win32/wic/-wic-about-windows-imaging-codec)

## Differences from other image decoding libraries

- Simple yet powerful API providing expected business entities - images, delays, palettes, pixels etc.
- Access to source pixel data w/o converting it to a different format (supported by the most codecs)
- Reading and writing images in multiple pixel formats, not only RGB and friends
- Access to the image properties w/o decoding the whole pixel data (probing)
- Access to the source image properties (source pixel format etc.)
- Image formats are supported through dynamically loaded codecs which means you can add or remove codecs
  without re-compiling the whole library
- Image codecs can be implemented in any programming language

## Development status

SAIL is ready for every day use. However, it's still under heavy development. The API can be changed at any time
breaking binary and source compatibility. Consider opening a GitHub [issue](https://github.com/smoked-herring/sail/issues)
if you have any feature requests or issue reports. Your help (pull requests etc.) is highly welcomed.

## Have questions or issues?

Opening a GitHub [issue](https://github.com/smoked-herring/sail/issues) is the preferred way
of communicating and solving problems.

See [FAQ](FAQ.md) for more.

## APIs overview

SAIL provides four levels of APIs, depending on your needs. Let's have a quick look at them.

### 1. `Junior`: I just want to load this damn JPEG from a file or memory

#### C:
```C
struct sail_image *image;

/*
 * sail_read_file() reads the image and outputs pixels in the BPP32-RGBA pixel format by default.
 * If SAIL is compiled with SAIL_READ_OUTPUT_BPP32_BGRA=ON, it outputs BPP32-BGRA pixels.
 * If you need to control output pixel formats, consider switching to the deep diver API.
 */
SAIL_TRY(sail_read_file(path, &image));

/*
 * Handle the image pixels here.
 * Use image->width, image->height, image->bytes_per_line,
 * image->pixel_format, and image->pixels for that.
 */

sail_destroy_image(image);
```

#### C++:
```C++
sail::image_reader reader;
sail::image image;

// read() reads the image and outputs pixels in the BPP32-RGBA pixel format by default.
// If SAIL is compiled with SAIL_READ_OUTPUT_BPP32_BGRA=ON, it outputs BPP32-BGRA pixels.
// If you need to control output pixel formats, consider switching to the deep diver API.
//
SAIL_TRY(reader.read(path, &image));

// Handle the image and its pixels here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.pixels() for that.
```

It's pretty easy, isn't it? :smile: See [EXAMPLES](EXAMPLES.md) for more.

### 2. `Advanced`: I want to load this damn animated GIF from a file or memory

See [EXAMPLES](EXAMPLES.md) for more.

### 3. `Deep diver`: I want to load this damn animated GIF from a file or memory and have control over selected codecs and output pixel formats

See [EXAMPLES](EXAMPLES.md) for more.

### 4. `Technical diver`: I want everything above and my custom I/O source

See [EXAMPLES](EXAMPLES.md) for more.

## Architecture overview

SAIL is written in pure C11 w/o using any third-party libraries (except for codecs). It also provides
bindings to C++.

### SAIL codecs

SAIL codecs is the deepest level. This is a set of standalone, dynamically loaded codecs (SO on Linux
and DLL on Windows). They implement actual decoding and encoding capabilities. End-users never work with
codecs directly. They always use abstract, high-level APIs for that.

Every codec is accompanied with a so called codec info (description) file which is just a plain text file.
It describes what the codec can actually do: what pixel formats it can read and output, what compression types
does it support, specifies a preferred output pixel format, and more.

By default, SAIL loads codecs on demand. To preload them, use `sail_init_with_flags(SAIL_FLAG_PRELOAD_CODECS)`.

### libsail-common

libsail-common holds common data types (images, pixel formats, I/O abstractions etc.) and a small set
of functions shared between SAIL codecs and the high-level APIs.

### libsail

libsail is a feature-rich, high-level API. It provides comprehensive and lightweight interfaces to decode
and encode images. End-users implementing C applications always work with libsail.

### libsail-c++

libsail-c++ is a C++ binding to libsail. End-users implementing C++ applications may choose
between libsail and libsail-c++. Using libsail-c++ is always recommended, as it's much more simple
to use in C++ applications.

## Building

Consider [EXAMPLES](EXAMPLES.md) after building and installing.

### CMake options overview

- `SAIL_DEV=ON|OFF` - Enable developer mode with pedantic warnings and possible `ASAN` enabled for examples. Default: `OFF`
- `SAIL_ONLY_CODECS="a;b;c"` - Enable only the codecs specified in this ';'-separated list.
  Codecs with missing dependencies will be disabled regardless this setting. Default: empty list
- `SAIL_EXCEPT_CODECS="a;b;c"` - Enable all codecs except the codecs specified in this ';'-separated list.
  Codecs with missing dependencies will be disabled regardless this setting. Default: empty list
- `SAIL_READ_OUTPUT_BPP32_BGRA=ON|OFF` - Make the read operations output BPP32-BGRA pixels instead of BPP32-RGBA. Default: `OFF`
- `SAIL_COLORED_OUTPUT=ON|OFF` - Enable colored console output on Windows >= 10 and Unix platforms. Default: `ON`

### Windows

#### Tested environments

- Windows 7 x64

#### Build requirements

- git
- cmake 3.6 or later
- MSVC 2017 or later

#### Build steps

Open `Git Bash` (installed along with `git`) and execute the following commands:

```
git clone --recursive https://github.com/smoked-herring/sail.git
cd sail

# Compile third-party dependencies
cd extra
./build
cd ..

# Compile SAIL
mkdir build
cd build
cmake -A x64 -DCMAKE_INSTALL_PREFIX="C:\SAIL" ..
cmake --build . --config Release

# Install
cmake --build . --config Release --target install
```

### macOS

#### Tested environments

- macOS 10.14 Mojave
- macOS 10.15 Catalina

#### Installation steps

```
brew install smoked-herring/sail/sail
```

Or

```
brew upgrade sail
```

### Linux

#### Tested environments

- LUbuntu 18.04 64-bit

#### Build requirements

- git
- cmake 3.6 or later
- pkg-config
- GCC and G++ 7.5 or later
- standard C/C++ development files installed (usually installed by metapackages like `build-essential`)
- codec-specific development libraries installed. You can grab the list from `debian/control`

#### Build steps

```
git clone --recursive https://github.com/smoked-herring/sail.git
cd sail

# Install the required dependencies grabbed from debian/control
sudo apt install ...

# Compile SAIL
mkdir build
cd build
cmake ..
make

# Install
... distro-specific installation
```

Debian rules are provided as well.

## License

Released under the MIT license.

```
Copyright (c) 2020 Dmitry Baryshev

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
