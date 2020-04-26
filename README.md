# SAIL: Squirrel Abstract Image Libraries

SAIL is a small and lightweight cross-platform library aimed to provide lightweight as well as complex APIs
to decode and encode images in different formats.

## APIs overview

SAIL provides 3 levels of APIs depending on your needs. Let's have a quick look at them.

### 1. `junior` "I just want to load this damn image"

**C**
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

**C++**
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

### 2. `advanced` "I want to load this damn image and have more control"

See [EXAMPLES](EXAMPLES.md) for more.

### 3. `deep diver` "I want to load this damn image and have comprehensive control"

See [EXAMPLES](EXAMPLES.md) for more.
