# SAIL: Squirrel Abstract Image Libraries

SAIL is a fast and lightweight cross-platform image decoding and encoding library providing multi-leveled APIs,
from one-liners to complex use cases with custom I/O sources. :sailboat:

SAIL is a fork of ksquirrel-libs, which was a set of C++ image codecs for the KSquirrel image viewer.
See [http://ksquirrel.sourceforge.net](http://ksquirrel.sourceforge.net).

Author: Dmitry Baryshev

## Target audience

- Image viewers.
- Game developers.
- Anyone who needs to load or save images in different image formats and who requires
  a lightweight and comprehensive API for that.

## Features overview

- Easy-to-use C and C++ interfaces
- Four levels of APIs, depending on your needs: `junior`, `advanced`, `deep diver`, and `technical diver`.
  See [EXAMPLES](EXAMPLES.md) for more.
- Reading images from file and memory
- Writing images to file and memory
- I/O abstraction for technical divers
- Reading operations are always able to output pixels in the `BPP24-RGB`
  and `BPP32-RGBA` formats. Supporting other output pixel formats is plugin-specific
- Reading operations are always able to output pixels in the **source pixel format**
  for those who want to kick the hell out of images manually. For example, one may want to work with raw
  `CMYK` pixels in a print image.
- Image formats are supported by dynamically loaded codecs (plugins)
- Read and write meta information like JPEG comments
- Easily extensible with new image formats for those who want to implement a specific codec for his/her needs
- Qt, SDL, and pure C examples

## Features NOT provided

- Image editing capabilities (filtering, distortion, scaling, etc.)
- Color space conversion functions
- Color management functions (applying ICC profiles etc.)
- EXIF rotation

## Supported image formats

1. [JPEG](https://wikipedia.org/wiki/JPEG) (reading and writing, requires `libjpeg-turbo`)
1. [PNG](https://wikipedia.org/wiki/Portable_Network_Graphics) (reading and writing, requires `libpng`)

## Supported platforms

Currently, SAIL supports the Windows and Linux platforms.

## Have questions or issues?

Opening a GitHub [issue](https://github.com/smoked-herring/sail/issues) is the preferred way
of communicating and solving problems.

## Architecture overview

SAIL is written in pure C11 w/o using any third-party libraries (except for codecs). It also provides
bindings to C++.

### SAIL plugins

SAIL plugins are the deepest level. This is a set of standalone, dynamically loaded codecs (SO on Linux
and DLL on Windows). They implement actual decoding and encoding capabilities. End-users never work with
plugins directly. They always use abstract, high-level APIs for that.

### libsail-common

libsail-common holds common data types (images, pixel formats, I/O abstractions etc.) and a small set
of functions shared between SAIL plugins and the high-level APIs.

### libsail

libsail is a feature-rich, high-level API. It provides comprehensive and lightweight interfaces to decode
and encode images. End-users implementing C applications always work with libsail.

### libsail-c++

libsail-c++ is a C++ binding to libsail. End-users implementing C++ applications may choose
between libsail and libsail-c++. Using libsail-c++ is always recommended, as it's much more simple
to use in C++ applications.

## License

- libsail-common, libsail, C++ bindings, and plugins are under LGPLv3+.
- Examples and tests are under the MIT license.

## APIs overview

SAIL provides four levels of APIs, depending on your needs. Let's have a quick look at them.

### 1. `Junior` - "I just want to load this damn JPEG."

#### C:
```C
struct sail_image *image;
unsigned char *image_bits;

/*
 * sail_read() reads the image and outputs pixels in BPP24-RGB pixel format for image formats
 * without transparency support and BPP32-RGBA otherwise. If you need to control output pixel
 * formats, consider switching to the deep diver API.
 */
SAIL_TRY(sail_read(path,
                   &image,
                   (void **)&image_bits));

/*
 * Handle the image bits here.
 * Use image->width, image->height, image->bytes_per_line,
 * and image->pixel_format for that.
 */

free(image_bits);
sail_destroy_image(image);
```

#### C++:
```C++
sail::image_reader reader;
sail::image image;

// read() reads the image and outputs pixels in BPP24-RGB pixel format for image formats
// without transparency support and BPP32-RGBA otherwise. If you need to control output pixel
// formats, consider switching to the deep diver API.
//
SAIL_TRY(reader.read(path, &image));

// Handle the image and its bits here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.bits() for that.
```

It's pretty easy, isn't it? :smile: See [EXAMPLES](EXAMPLES.md) for more.

### 2. `Advanced` - "I want to load this damn animated GIF."

See [EXAMPLES](EXAMPLES.md) for more.

### 3. `Deep diver` - "I want to load this damn possibly multi-paged image from memory and have comprehensive control over selected plugins and output pixel formats."

See [EXAMPLES](EXAMPLES.md) for more.

### 4. `Technical diver` - "I want everything above and my custom I/O source."

See [EXAMPLES](EXAMPLES.md) for more.

## Building

### CMake options overview

- `SAIL_DEV=ON|OFF` - Enable developer mode with pedantic warnings and possible `ASAN` enabled for examples
- `SAIL_ONLY_PLUGINS="a;b;c"` - Enable only the plugins specified in this ';'-separated list.
  Plugins with missing dependencies will be disabled regardless this setting
- `SAIL_EXCEPT_PLUGINS="a;b;c"` - Enable all plugins except the plugins specified in this ';'-separated list.
  Plugins with missing dependencies will be disabled regardless this setting
- `SAIL_COLORED_OUTPUT=ON|OFF` - Enable colored console output on Windows >= 10 and Unix platforms

### Windows

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

### Linux

#### Build requirements

- git
- cmake 3.6 or later
- GCC and G++ 7.5 or later
- standard C/C++ development files installed (usually installed by metapackages like `build-essential`)
- codec-specific development libraries installed. You can grab the list from `debian/control`

#### Build steps

```
git clone --recursive https://github.com/smoked-herring/sail.git
cd sail

# Compile SAIL
mkdir build
cd build
cmake ..
make

# Install
... distro-specific installation
```

Debian rules are provided as well.
