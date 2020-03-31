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

/* Read or writeoptions. */
enum SailIoOptions {

    /* Read or write image meta information like JPEG comments. */
    SAIL_IO_OPTION_META_INFO = 1 << 0,

    /* Not to be used. Resize the enum for future elements. */
    SAIL_IO_OPTIONS_RESIZE_ENUM_TO_INT = INT_MAX
};

/*
 * A structure representing a file object.
 */
struct sail_file {

    /* File descriptor. */
    FILE *fptr;

    /*
     * Plugin-specific data. A plugin could set pimpl to its plugin-specific data storage and access it
     * in read or write functions. Will be destroyed automatically in sail_file_destroy().
     */
    void *pimpl;
};

struct sail_meta_entry_node;

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

    /* Image meta information. See sail_meta_entry_node. */
    struct sail_meta_entry_node *meta_entry_node;

    /* Image properties. See SailImageProperties. */
    int properties;

    /* Image source pixel format. See SailPixelFormat. */
    int source_pixel_format;
};

/* Options to modify reading operations. */
struct sail_read_options {

    /*
     * Hint to modify output pixel format. This is just a hint, not an obligation. Plugin (or an underlying codec)
     * may reject the requested pixel format.
     *
     * NOTE: SAIL doesn't provide pixel format conversion capabilities. This hist is passed to an underlying codec
     * which may reject or ignore it. The actual selected pixel format will be available in sail_image.
     */
    int pixel_format;

    /* IO manipulation options. See SailIoOptions. */
    int options;
};

/* Options to modify writing operations. */
struct sail_write_options {

    /*
     * Hint to modify output pixel format. This is just a hint, not an obligation. Plugin (or an underlying codec)
     * may reject the requested pixel format.
     *
     * NOTE: SAIL doesn't provide pixel format conversion capabilities. This hist is passed to an underlying codec
     * which may reject or ignore it.
     */
    int pixel_format;

    /* IO manipulation options. See SailIoOptions. */
    int options;
};

/*
 * File functions.
 */

/*
 * Opens the specified image file using the specified mode (as in fopen). The assigned file MUST be destroyed later
 * with sail_file_destroy().
 *
 * Returns 0 on success or errno on error.
 */
int SAIL_EXPORT sail_file_alloc(const char *filepath, const char *mode, struct sail_file **file);

/*
 * Closes the specified file and destroys all its internal memory buffers. Does nothing if the file is already closed.
 * The "file" pointer MUST NOT be used anymore after calling this function.
 */
void SAIL_EXPORT sail_file_destroy(struct sail_file *file);

/*
 * Image functions.
 */

/*
 * Allocates a new image. The assigned image MUST be destroyed later with sail_image_destroy().
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
 * Options functions.
 */

/*
 * Allocates default read options. The assigned read options MUST be destroyed later
 * with sail_read_options_destroy().
 *
 * Default read options:
 *     - pixel format: source
 *     - options: read meta info
 */
int SAIL_EXPORT sail_read_options_alloc(struct sail_read_options **read_options);

/*
 * Destroys the specified read options and all its internal allocated memory buffers.
 * The "read_options" pointer MUST NOT be used after calling this function.
 */
void SAIL_EXPORT sail_read_options_destroy(struct sail_read_options *read_options);

/*
 * Allocates default write options. The assigned write options MUST be destroyed later
 * with sail_write_options_destroy().
 *
 * Default write options:
 *     - pixel format: source
 *     - options: write meta info
 */
int SAIL_EXPORT sail_write_options_alloc(struct sail_write_options **write_options);

/*
 * Destroys the specified write options and all its internal allocated memory buffers.
 * The "write_options" pointer MUST NOT be used after calling this function.
 */
void SAIL_EXPORT sail_write_options_destroy(struct sail_write_options *write_options);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
