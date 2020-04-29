# SAIL: Squirrel Abstract Image Libraries

SAIL is a fast and lightweight cross-platform library aimed to provide lightweight as well as complex APIs
to decode and encode images in different formats. :sailboat:

Author: Dmitry Baryshev.

## Target audience

- Game developers.
- Image viewers.
- Anyone who needs loading or saving images in different image formats.

## Features overview

- Reading and writing images in numerous plugin-specific pixel formats. For example, the JPEG plugin
  is able to read `RGB` and `YCbCr` images and output them to memory as `Grayscale` pixels and vice versa.
- Reading images and outputting them to memory in source (raw) pixel format for those who want to kick the hell
  out of images manually. For example, one may want to work with raw `CMYK` pixels in a printing image.
  :warning: Some plugins might not support outputting source pixels.
- easy-to-use C and C++ interfaces.
- 4 levels of APIs depending on your needs: `junior`, `advanced`, `deep diver`, and `technical diver`. See [EXAMPLES](EXAMPLES.md) for more.
- I/O abstraction for technical divers.
- Image formats are supported by dynamically loaded codecs (plugins).
- Easily extensible with new image formats for those who want to implement a specific codec for his/her needs.

## Features NOT provided

- image editing capabilities (filtering, distortion, scaling etc.).
- color space conversion functions.
- color management functions (applying ICC profiles etc.).
- EXIF rotation.

## Image formats supported

- JPEG (requires `libjpeg-turbo`)

## Supported platforms

Currently SAIL supports Windows and Linux platforms.

## License

- libsail-common, libsail, C++ bindings, and plugins are under LGPLv3+.
- Examples are under the MIT license.

## APIs overview

SAIL provides 3 levels of APIs depending on your needs. Let's have a quick look at them.

### 1. `junior` "I just want to load this damn JPEG"

#### C:
```C
struct sail_image *image;
unsigned char *image_bits;

/*
 * sail_read() reads the image and outputs pixels in RGB pixel format for image formats
 * without transparency support and RGBA otherwise.
 */
SAIL_TRY(sail_read(path,
                   &image,
                   reinterpret_cast<void **>(&image_bits)));

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
// without transparency support and RGBA otherwise.
//
SAIL_TRY(reader.read(path, &image));

// Handle the image and its bits here.
// Use image->width(), image->height(), image->bytes_per_line(),
// image->pixel_format(), and image->bits() for that.
```

It's pretty easy, isn't it? :smile: See [EXAMPLES](EXAMPLES.md) for more.

### 2. `advanced` "I want to load this damn animated GIF"

See [EXAMPLES](EXAMPLES.md) for more.

### 3. `deep diver` "I want to load this damn possibly multi-paged image from memory and have comprehensive control over selected plugins and output pixel formats"

See [EXAMPLES](EXAMPLES.md) for more.

### 4. `technical diver` "I want everything above and my custom I/O source"

See [EXAMPLES](EXAMPLES.md) for more.
