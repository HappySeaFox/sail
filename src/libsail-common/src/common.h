#ifndef SAIL_COMMON_H
#define SAIL_COMMON_H

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

/*
 * Common data structures and functions used across SAIL, both in libsail and in image plugins.
 */

/* Pixel format */
enum SailPixelFormat {

    /* Unsupported pixel format that cannot be read or written. */
    SAIL_PIXEL_FORMAT_UNSUPPORTED,

    /* 
     * Don't manipulate the output image data. Copy it as is from the source file.
     * The caller will handle the returned pixel data manually.
     */
    SAIL_PIXEL_FORMAT_SOURCE,

    SAIL_PIXEL_FORMAT_MONO_BE,
    SAIL_PIXEL_FORMAT_MONO_LE,

    SAIL_PIXEL_FORMAT_GRAYSCALE8,
    SAIL_PIXEL_FORMAT_GRAYSCALE16,
    SAIL_PIXEL_FORMAT_INDEXED8,

    SAIL_PIXEL_FORMAT_BGR30,
    SAIL_PIXEL_FORMAT_RGB16,
    SAIL_PIXEL_FORMAT_RGB30,
    SAIL_PIXEL_FORMAT_RGB32,
    SAIL_PIXEL_FORMAT_RGB444,
    SAIL_PIXEL_FORMAT_RGB555,
    SAIL_PIXEL_FORMAT_RGB666,
    SAIL_PIXEL_FORMAT_RGB888,
    SAIL_PIXEL_FORMAT_RGBA8888,
    SAIL_PIXEL_FORMAT_ARGB8888,

    /* Not to be used. Resize the enum for future elements. */
    SAIL_PIXEL_FORMAT_RESIZE_ENUM_TO_INT = INT_MAX
};

/*
 * Color space. SAIL doesn't provide color space conversion capabilities.
 * This enum is used to provide a source color space for a user. Image data
 * is always converted to RGB color space.
 */
enum SailColorSpace {

    /* Unknown color space. */
    SAIL_COLOR_SPACE_UNKNOWN,

    SAIL_COLOR_SPACE_GRAYSCALE,
    SAIL_COLOR_SPACE_RGB,
    SAIL_COLOR_SPACE_YCBCR,
    SAIL_COLOR_SPACE_CMYK,
    SAIL_COLOR_SPACE_YCCK,

    /* Not to be used. Resize the enum for future elements. */
    SAIL_COLOR_SPACE_RESIZE_ENUM_TO_INT = INT_MAX
};

/* Image properties. */
enum SailImageProperties {
    SAIL_IMAGE_FLIPPED_VERTICALLY = 1 << 0,

    /* Not to be used. Resize the enum for future elements. */
    SAIL_IMAGE_PROPERTY_RESIZE_ENUM_TO_INT = INT_MAX
};

/*
 * A structure representing a file object.
 */
struct sail_file {

    /* File descriptor */
    FILE *fptr;

    /* Plugin-specific data. MUST be destroyed in every plugin upon reading or writing finialization. */
    void *pimpl;
};

/*
 * A structure representing an image.
 */
struct sail_image {

    /* Image width */
    int width;

    /* Image height */
    int height;

    /* Image pixel format. See SailPixelFormat. */
    int pixel_format;

    /* Number of passes needed to read or write an entire image frame */
    int passes;

    /* Is the image a frame in an animation */
    bool animated;

    /* Delay in milliseconds if the image is a frame in an animation */
    int delay;

    /* Palette pixel format */
    int palette_pixel_format;

    /* Palette if a "source" pixel format was chosen and the image has a palette */
    void *palette;

    /* Size of the palette data in bytes */
    int palette_size;

    /* Image source pixel format. See SailPixelFormat. */
    int source_pixel_format;

    /* Image source color space. See SailColorSpace. */
    int source_solor_space;
};

/* Options to modify reading operations. */
struct sail_read_options {

    /* Modify output pixel format. */
    int pixel_format;
};

/* Options to modify writing operations. */
struct sail_write_options {

    /* Modify output pixel format. */
    int pixel_format;
};

/*
 * File functions.
 */

/*
 * Opens the specified image file using the specified mode (as in fopen). The assigned file MUST be closed later
 * with sail_file_close().
 *
 * Returns 0 on success or errno on error.
 */
int sail_file_open(const char *filepath, const char *mode, struct sail_file **file);

/*
 * Closes the specified file. Does nothing if the file is already closed.
 * The "file" pointer MUST NOT be used after calling this function.
 */
void sail_file_close(struct sail_file *file);

/*
 * Image functions.
 */

/*
 * Destroys the specified image and all its internal allocated memory buffers.
 * The "image" pointer MUST NOT be used after calling this function.
 */
void sail_image_destroy(struct sail_image *image);

#endif
