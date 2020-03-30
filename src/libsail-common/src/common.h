#ifndef SAIL_COMMON_H
#define SAIL_COMMON_H

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef SAIL_BUILD
    #include "export.h"
#else
    #include <sail/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Common data structures and functions used across SAIL, both in libsail and in image plugins.
 */

/* Pixel format */
enum SailPixelFormat {

    /* Unknown pixel format that cannot be parsed by SAIL. */
    SAIL_PIXEL_FORMAT_UNKNOWN,

    /*
     * Don't manipulate the output image data. Copy it as is from the source file.
     * The caller will handle the returned pixel data manually.
     */
    SAIL_PIXEL_FORMAT_SOURCE,

    SAIL_PIXEL_FORMAT_MONO,

    SAIL_PIXEL_FORMAT_GRAYSCALE,
    SAIL_PIXEL_FORMAT_INDEXED,

    SAIL_PIXEL_FORMAT_YCBCR,
    SAIL_PIXEL_FORMAT_CMYK,
    SAIL_PIXEL_FORMAT_YCCK,
    SAIL_PIXEL_FORMAT_RGB,
    SAIL_PIXEL_FORMAT_RGBX,
    SAIL_PIXEL_FORMAT_BGR,
    SAIL_PIXEL_FORMAT_BGRX,
    SAIL_PIXEL_FORMAT_XRGB,
    SAIL_PIXEL_FORMAT_XBGR,
    SAIL_PIXEL_FORMAT_RGBA,
    SAIL_PIXEL_FORMAT_BGRA,
    SAIL_PIXEL_FORMAT_ARGB,
    SAIL_PIXEL_FORMAT_ABGR,
    SAIL_PIXEL_FORMAT_RGB565,

    /* Not to be used. Resize the enum for future elements. */
    SAIL_PIXEL_FORMAT_RESIZE_ENUM_TO_INT = INT_MAX
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
};

/* Options to modify reading operations. */
struct sail_read_options {

    /* Modify output pixel format. */
    int pixel_format;

    /* Read any supported meta-info from the file. For example, JPEG comments. */
    bool meta_info;
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
int SAIL_EXPORT sail_file_open(const char *filepath, const char *mode, struct sail_file **file);

/*
 * Closes the specified file. Does nothing if the file is already closed.
 * The "file" pointer MUST NOT be used after calling this function.
 */
void SAIL_EXPORT sail_file_close(struct sail_file *file);

/*
 * Image functions.
 */

/*
 * Allocates a nw image. The assigned image MUST be destroyed later with sail_image_destroy().
 *
 * Returns 0 on success or errno on error.
 */
int SAIL_EXPORT sail_image_alloc(struct sail_image **image);

/*
 * Destroys the specified image and all its internal allocated memory buffers.
 * The "image" pointer MUST NOT be used after calling this function.
 */
void SAIL_EXPORT sail_image_destroy(struct sail_image *image);

/*
 * Returns default read options.
 */
struct sail_read_options SAIL_EXPORT sail_default_read_options();

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
