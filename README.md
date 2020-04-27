# SAIL: Squirrel Abstract Image Libraries

SAIL is a small and lightweight cross-platform library aimed to provide lightweight as well as complex APIs
to decode and encode images in different formats. :sailboat:

## Features

- easy-to-use C and C++ interfaces.
- 3 levels of APIs depending on your needs: `junior`, `advanced`, and `deep diver`. See [EXAMPLES](EXAMPLES.md) for more.
- Reading and writing images in numerous plugin-specific pixel formats. For example, the JPEG plugin
  is able to read `RGB` and `YCbCr` images and output them to memory as `Grayscale` pixels and vice versa.
- Reading images and outputting them to memory in source pixel format without conversion for those who want
  to kick the hell out of images manually. :warning: Some plugins might not support outputting source pixels.
- Image formats are supported by dynamically loaded codecs (plugins).
- Easily extensible with new image formats for those who want to implement a specific codec for his/her needs.

## Supported platforms

Currently SAIL supports Windows and Linux platforms.

## APIs overview

SAIL provides 3 levels of APIs depending on your needs. Let's have a quick look at them.

### 1. `junior` "I just want to load this damn JPEG"

#### C:
```C
struct sail_image *image;
unsigned char *image_bits;

/*
 * sail_read() reads the image and outputs pixels in a plugin-specific preferred pixel format.
 */
SAIL_TRY_OR_CLEANUP(sail_read(path,
                              &image,
                              reinterpret_cast<void **>(&image_bits)),
                    /* cleanup */ free(image_bits),
                                  sail_destroy_image(image));

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

// read() reads the image and outputs pixels in a plugin-specific preferred pixel format.
//
SAIL_TRY(reader.read(path, &image));

// Handle the image and its bits here.
// Use image->width(), image->height(), image->bytes_per_line(),
// image->pixel_format(), and image->bits() for that.
```

It's pretty easy, isn't it? :smile: See [EXAMPLES](EXAMPLES.md) for more.

### 2. `advanced` "I want to load this damn animated GIF"

See [EXAMPLES](EXAMPLES.md) for more.

### 3. `deep diver` "I want to load this damn possibly miulti-paged image and have comprehensive control over selected plugins and output pixel formats"

See [EXAMPLES](EXAMPLES.md) for more.
