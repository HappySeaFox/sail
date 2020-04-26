## APIs overview

SAIL provides 3 levels of APIs depending on your needs. Let's have a look at them.

### 1. `junior` "I just want to load this damn image"

**C:**
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

**C++:**
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

### 2. `advanced` "I want to load this damn image and have more control"

**C:**
```C
sail_context *context = NULL;

/*
 * Initialize SAIL context. Needed only once.
 */
SAIL_TRY(sail_init(&context));

/*
 * A local void pointer is needed for SAIL to save a state in it.
 */
void *pimpl = NULL;
struct sail_image *image = NULL;
unsigned char *image_bits = NULL;

/*
 * Starts reading the specified file.
 * The subsequent calls to sail_read_next_frame() will output pixels
 * in a plugin-specific preferred pixel format.
 */
SAIL_TRY_OR_CLEANUP(sail_start_reading(path.toLocal8Bit(), d->context, NULL, &pimpl),
                    /* cleanup */ sail_stop_reading(pimpl),
                                  free(image_bits),
                                  sail_destroy_image(image));

/*
 * Read just a single frame. It's possible to read more frame if any. Just continue
 * reading frames until sail_read_next_frame() returns 0. If no more frames are available,
 * it returns SAIL_NO_MORE_FRAMES.
 */
SAIL_TRY_OR_CLEANUP(sail_read_next_frame(pimpl, &image, (void **)&image_bits),
                    /* cleanup */ sail_stop_reading(pimpl),
                                  free(image_bits),
                                  sail_destroy_image(image));

/*
 * It's essential to ALWAYS stop reading to free memory resources.
 * Avoiding doing so will lead to memory leaks.
 */
SAIL_TRY_OR_CLEANUP(sail_stop_reading(pimpl),
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

**C++:**
```C++
// Create a new SAIL context.
// You can re-use the same context for multiple readings or writings.
//
sail::context context;

sail::image_reader reader(&context);
sail::image image;

// It's essential to ALWAYS stop reading to free memory resources.
// Avoiding doing so will lead to memory leaks. This code gets executed
// when the outer scope exits.
//
SAIL_AT_SCOPE_EXIT (
    reader.stop_reading();
);

// Starts reading the specified file.
// The subsequent calls to read_next_frame() will output pixels in a plugin-specific
// preferred pixel format.
//
SAIL_TRY(reader.start_reading(path.toLocal8Bit().constData()));

// Read just a single frame. It's possible to read more frame if any. Just continue
// reading frames until sail_read_next_frame() returns 0. If no more frames are available,
// it returns SAIL_NO_MORE_FRAMES.
//
// read_next_frame() outputs pixels in a plugin-specific preferred pixel format.
//
SAIL_TRY_OR_CLEANUP(reader.read_next_frame(&image));

SAIL_TRY(reader.stop_reading());

// Handle the image and its bits here.
// Use image->width(), image->height(), image->bytes_per_line(),
// image->pixel_format(), and image->bits() for that.
```

### 3. `deep diver` "I want to load this damn image and have comprehensive control"

**C:**
```C
```

**C++:**
```C++
```
