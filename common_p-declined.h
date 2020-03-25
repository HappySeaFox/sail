#ifndef SAIL_COMMON_P_H
#define SAIL_COMMON_P_H

/*
 * WARNING: This file is not a part of the public SAIL API. Do not include it in real projects.
 */

/*
 * A structure representing a file object.
 */
struct sail_file
{
    /* File descriptor */
    int fd;

    /* Plugin-specific data */
    void *pimpl;

    /* Plugin-specific data deleter */
    void (*pimpl_destroy)(void *);
};

/*
 * A structure representing an image.
 */
struct sail_image
{
    /* Image width */
    int width;

    /* Image height */
    int height;

    /* Image pixel format. See SailPixelFormat. */
    int pixel_format;

    /* Number of passes in this image */
    int passes;

    /* Is the image an animation */
    bool animated;

    /* Delay in milliseconds if the image is a frame in an animation */
    int delay;

    /* Palette pixel format */
    int palette_pixel_format;

    /* Palette if a "source" pixel format was chosen and the image has a palette */
    void *palette;

    /* Size of the palette data in bytes */
    int palette_size;
};

#endif
