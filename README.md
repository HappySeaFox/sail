# SAIL: Squirrel Abstract Image Libraries

SAIL is a fast and lightweight cross-platform image decoding and encoding library providing multi-leveled APIs
from one-liners to complex use-cases with custom I/O sources. :sailboat:

SAIL is a fork of ksquirrel-libs which was a set of C++ image codecs for KSquirrel image viewer.
See [http://ksquirrel.sourceforge.net](http://ksquirrel.sourceforge.net).

Author: Dmitry Baryshev.

## Target audience

- Game developers.
- Image viewers.
- Anyone who needs loading or saving images in different image formats and having lightweight and comprehensive API for that.

## Features overview

- easy-to-use C and C++ interfaces.
- 4 levels of APIs depending on your needs: `junior`, `advanced`, `deep diver`, and `technical diver`.
  See [EXAMPLES](EXAMPLES.md) for more.
- Reading images from file and memory.
- Writing images to file and memory.
- I/O abstraction for technical divers.
- Image formats are supported by dynamically loaded codecs (plugins).
- It's guaranteed that every plugin is able to read and output to memory pixels in `RGB` and `RGBA` formats.
  Supporting other output pixel formats is plugin-specific.
- Reading and writing images in numerous plugin-specific pixel formats. For example, the JPEG plugin
  is able to read `RGB` and `YCbCr` images and output them to memory as `Grayscale` pixels and vice versa.
- Reading images and outputting them to memory in source (raw) pixel format for those who want to kick the hell
  out of images manually. For example, one may want to work with raw `CMYK` pixels in a printing image.
  :warning: Some plugins might not support outputting source pixels.
- Reading and writing meta information like JPEG comments.
- Easily extensible with new image formats for those who want to implement a specific codec for his/her needs.

## Features NOT provided

- image editing capabilities (filtering, distortion, scaling etc.).
- color space conversion functions.
- color management functions (applying ICC profiles etc.).
- EXIF rotation.

## Image formats supported

- [JPEG](https://wikipedia.org/wiki/JPEG) (reading and writing, requires `libjpeg-turbo`)

## Supported platforms

Currently SAIL supports Windows and Linux platforms.

## Architecture overview

SAIL is written in C w/o using any third-party libraries (except for codecs). It also provides bindings to C++.

### SAIL plugins

SAIL plugins is the deepest level. It's a set of standalone dynamically loaded codecs (SO on Linux and DLL on Windows).
They implement actual decoding and encoding capabilities. End-users never work with plugins directly.
They always use abstract high-level APIs for that.

### libsail-common

libsail-common holds common data types (images, pixel formats, I/O abstractions etc.) and a small set of functions
shared between SAIL plugins and the high-level APIs.

### libsail

libsail is a feature-rich high-level API. It provides comprehensive and lightweight interfaces to decode and encode images.
End-users implementing C applications always work with libsail.

### libsail-c++

libsail-c++ is a C++ binding to libsail. End-users implementing C++ applications may choose
between libsail and libsail-c++. Using libsail-c++ is always recommended as it's much more simple
to use in C++ applications.

## License

- libsail-common, libsail, C++ bindings, and plugins are under LGPLv3+.
- Examples are under the MIT license.

## APIs overview

SAIL provides 4 levels of APIs depending on your needs. Let's have a quick look at them.

### 1. `junior` "I just want to load this damn JPEG"

#### C:
```C
struct sail_image *image;
unsigned char *image_bits;

/*
 * sail_read() reads the image and outputs pixels in RGB pixel format for image formats
 * without transparency support and RGBA otherwise. If you need to control output pixel
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

// read() reads the image and outputs pixels in RGB pixel format for image formats
// without transparency support and RGBA otherwise. If you need to control output pixel
// formats, consider switching to the deep diver API.
//
SAIL_TRY(reader.read(path, &image));

// Handle the image and its bits here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.bits() for that.
```

It's pretty easy, isn't it? :smile: See [EXAMPLES](EXAMPLES.md) for more.

### 2. `advanced` "I want to load this damn animated GIF"

See [EXAMPLES](EXAMPLES.md) for more.

### 3. `deep diver` "I want to load this damn possibly multi-paged image from memory and have comprehensive control over selected plugins and output pixel formats"

See [EXAMPLES](EXAMPLES.md) for more.

### 4. `technical diver` "I want everything above and my custom I/O source"

See [EXAMPLES](EXAMPLES.md) for more.
