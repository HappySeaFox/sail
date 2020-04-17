/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SAIL_COMMON_H
#define SAIL_COMMON_H

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
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

    /* Image needs flipping vertically. */
    SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY = 1 << 0,

    /* Image is interlaced. */
    SAIL_IMAGE_PROPERTY_INTERLACED         = 1 << 1,

    /* Not to be used. Resize the enum for future elements. */
    SAIL_IMAGE_PROPERTIES_RESIZE_ENUM_TO_INT = INT_MAX
};

/* Pixels compression types. */
enum SailCompression {

    /* RLE compression. */
    SAIL_COMPRESSION_RLE = 1 << 0,
};

/* Plugin features. */
enum SailPluginFeatures {

    /* Ability to read or write static images. */
    SAIL_PLUGIN_FEATURE_STATIC     = 1 << 0,

    /* Ability to read or write animated images. */
    SAIL_PLUGIN_FEATURE_ANIMATED   = 1 << 1,

    /* Ability to read or write multipaged (not animated) images. */
    SAIL_PLUGIN_FEATURE_MULTIPAGED = 1 << 2,

    /* Ability to read or write simple image meta information like JPEG comments. */
    SAIL_PLUGIN_FEATURE_META_INFO  = 1 << 3,

    /* Ability to read or write EXIF meta information. */
    SAIL_PLUGIN_FEATURE_EXIF       = 1 << 4,

    /* Ability to read or write interlaced images. */
    SAIL_PLUGIN_FEATURE_INTERLACED = 1 << 5,

    /* Not to be used. Resize the enum for future elements. */
    SAIL_PLUGIN_FEATURES_RESIZE_ENUM_TO_INT = INT_MAX
};

/* Read or write options. */
enum SailIoOptions {

    /* Instruction to read or write simple image meta information like JPEG comments. */
    SAIL_IO_OPTION_META_INFO = 1 << 0,

    /* Instruction to read or write EXIF meta information. */
    SAIL_IO_OPTION_EXIF      = 1 << 1,

    /* Not to be used. Resize the enum for future elements. */
    SAIL_IO_OPTIONS_RESIZE_ENUM_TO_INT = INT_MAX
};

struct sail_meta_entry_node;

/*
 * A structure representing an image. Fields set when reading images are marked with READ.
 * Fields that must be set by a caller when writing images are marked with WRITE.
 */
struct sail_image {

    /*
     * Image width.
     *
     * READ:  Set by SAIL to a positive image width in pixels.
     * WRITE: Must be set by a caller to a positive image width in pixels.
     */
    int width;

    /*
     * Image height.
     *
     * READ:  Set by SAIL to a positive image height in pixels.
     * WRITE: Must be set by a caller to a positive image height in pixels.
     */
    int height;

    /*
     * Bytes per line. Some image formats (like BMP) pad rows of pixels to some boundary.
     *
     * READ:  Set by SAIL to a positive length of a row of pixels in bytes.
     * WRITE: Ignored.
     */
    int bytes_per_line;

    /*
     * Image pixel format. See SailPixelFormat.
     *
     * READ:  Set by SAIL to a valid output image pixel format. The list of supported output pixel formats
     *        by this plugin could be obtained from sail_read_features.input_pixel_formats.
     * WRITE: Must be set by a caller to a valid input image pixel format. Pixels in this format will be supplied
     *        to the plugin by a caller later. The list of supported input pixel formats by this plugin
     *        could be obtained from sail_write_features.output_pixel_formats.
     */
    int pixel_format;

    /*
     * Number of passes needed to read or write an entire image frame. 1 by default.
     *
     * READ:  Set by SAIL to a positive number of passes needed to read an image. For example, interlaced PNGs
     *        have 8 passes.
     * WRITE: Ignored. Use sail_write_features.passes to determine the actual number of passes needed to write
     *        an interlaced image.
     */
    int passes;

    /*
     * Is the image a frame in an animation.
     *
     * READ:  Set by SAIL to true if the image is a frame in an animation.
     * WRITE: Ignored.
     */
    bool animated;

    /*
     * Delay in milliseconds if the image is a frame in an animation or 0 otherwise.
     *
     * READ:  Set by SAIL to a non-negative number of milliseconds.
     * WRITE: Must be set by a caller to a non-negative number of milliseconds.
     */
    int delay;

    /*
     * Palette pixel format.
     *
     * READ:  Set by SAIL to a valid palette pixel format if the image is indexed (palette is not NULL).
     * WRITE: Must be set by a caller to a valid palette pixel format if the image is indexed
     *        (palette is not NULL).
     */
    int palette_pixel_format;

    /*
     * Palette if the image has a palette and the requested pixel format assumes having a palette.
     * Destroyed by sail_destroy_image().
     *
     * READ:  Set by SAIL to a valid pixel array if the image is indexed.
     * WRITE: Must be set allocated and set by a caller to a valid pixel array if the image is indexed.
     */
    void *palette;

    /*
     * Size of the palette data in bytes.
     *
     * READ:  Set by SAIL to a valid palette size in bytes if the image is indexed or to 0.
     * WRITE: Must be set by a caller to a valid palette size in bytes if the image is indexed.
     */
    int palette_size;

    /*
     * Image meta information. See sail_meta_entry_node. Plugins guarantee that keys and values are non-NULL.
     * Destroyed by sail_destroy_image().
     *
     * READ:  Set by SAIL to a valid linked list with simple meta information (like JPEG comments) or to NULL.
     * WRITE: Must be set allocated and set by a caller to a valid linked list with simple meta information
     *        like JPEG comments if the image have it.
     */
    struct sail_meta_entry_node *meta_entry_node;

    /*
     * Decoded image properties. See SailImageProperties.
     *
     * READ:  Set by SAIL to a valid image properties. For example, some image formats store images flipped.
     *        A caller must use this field to manipulate the output image accordingly (e.g. flip back etc.).
     * WRITE: Ignored.
     */
    int properties;

    /*
     * Image source pixel format. See SailPixelFormat.
     *
     * READ:  Set by SAIL to a valid source image pixel format before converting it to a requested pixel format
     *        with sail_read_options.pixel_format.
     * WRITE: Ignored.
     */
    int source_pixel_format;

    /*
     * Image source properties. See SailImageProperties.
     *
     * READ:  Set by SAIL to a valid source image properties or to 0.
     * WRITE: Ignored.
     */
    int source_properties;
};

typedef struct sail_image sail_image_t;

/*
 * Read features. Use this structure to determine what a plugin can actually read.
 */
struct sail_read_features {

    /*
     * A list of supported pixel formats that can be read by this plugin. One of these values
     * will be stored in sail_image.source_pixel_format. See SailPixelFormat.
     *
     * For example: CMYK, YCBCR, RGB.
     *
     * NOTE: Some input pixel formats might not map to some output pixel formats.
     *       Let's take a look at an hypothetical example:
     *
     * A hypothetical SAIL plugin supports input YCBCR images and it's able to output RGB pixel data from them.
     * Additionally, it supports input CMYK images and it's able to output YCCK pixel data from them.
     * So the full conversion table with all possible input/output variants looks like that:
     *
     * [ Read from file ] YCBCR => RGB  [ Output to memory ]
     * [ Read from file ] CMYK  => YCCK [ Output to memory ]
     *
     * sail_read_features.input_pixel_formats will contain YCBCR and CMYK pixel formats.
     * sail_read_features.output_pixel_formats will contain RGB and YCCK pixel formats.
     *
     * However, if you try to read an YCBCR image and output YCCK pixels, the codec will return
     * an error.
     */
    int *input_pixel_formats;

    /* The length of input_pixel_formats. */
    int input_pixel_formats_length;

    /*
     * A list of supported pixel formats that can be outputted by this plugin.
     *
     * It's not guaranteed that every input pixel format in input_pixel_formats could be converted
     * to every output pixel format in output_pixel_formats. Some could be converted and some not.
     * See the comments above.
     *
     * If the array contains SAIL_PIXEL_FORMAT_SOURCE, then the codec is able to output raw pixel data.
     * It's a caller's responsibility to convert it to a suitable format then. Refer to sail_image.pixel_format
     * to detect the actual pixel format of the raw data in this case.
     *
     * For example: SOURCE, RGB, YCCK.
     */
    int *output_pixel_formats;

    /* The length of output_pixel_formats. */
    int output_pixel_formats_length;

    /* Output_pixel format to use by default. */
    int preferred_output_pixel_format;

    /* Supported plugin features of reading operations. See SailPluginFeatures. */
    int features;
};

typedef struct sail_read_features sail_read_features_t;

/*
 * Write features. Use this structure to determine what a plugin can actually write.
 */
struct sail_write_features {

    /*
     * A list of supported input pixel formats that can be passed to this plugin from a caller.
     * One of these values could be specified in sail_image.pixel_format. See SailPixelFormat.
     *
     * For example: CMYK, YCBCR, RGB.
     *
     * NOTE: Some input pixel formats might not map to some output pixel formats.
     *       Let's take a look at an hypothetical example:
     *
     * A hypothetical SAIL plugin supports input RGB pixel data and it's able to output YCBCR files from it.
     * Additionally, it supports input YCCK pixel data and it's able to output CMYK files from it.
     * So the full conversion table with all possible input/output variants looks like that:
     *
     * [ Read from memory ] RGB   => YCBCR [ Output to file ]
     * [ Read from memory ] YCCK  => CMYK  [ Output to file ]
     *
     * sail_write_features.input_pixel_formats will contain RGB and YCCK pixel formats.
     * sail_write_features.output_pixel_formats will contain YCBCR and CMYK pixel formats.
     *
     * However, if you try to write a YCBCR file from YCCK pixel data, the codec will return
     * an error.
     */
    int *input_pixel_formats;

    /* The length of input_pixel_formats. */
    int input_pixel_formats_length;

    /*
     * A list of supported pixel formats that can be outputted by this plugin to a file.
     *
     * It's not guaranteed that every input pixel format in input_pixel_formats could be converted
     * to every output pixel format in output_pixel_formats. Some could be converted and some not.
     *
     * For example: CMYK, YCBCR, RGB.
     */
    int *output_pixel_formats;

    /* The length of output_pixel_formats. */
    int output_pixel_formats_length;

    /* Output_pixel format to use by default. */
    int preferred_output_pixel_format;

    /* Supported plugin features of writing operations. See SailPluginFeatures. */
    int features;

    /*
     * Required image properties. For example, in input image must be flipped by a caller before writing
     * it with SAIL (or supply scan lines in a reverse order). See SailImageProperties.
     */
    int properties;

    /* Number of passes to write an interlaced image or 0. */
    int passes;

    /*
     * A list of supported pixels compression types by this plugin. NULL if no compression types are available.
     * In most cases plugins support compression levels or compression types, but not both.
     *
     * For example:
     *
     *     1. The JPEG plugin supports only compression levels (compression_min, compression_max, compression_default).
     *     2. The TIFF plugin supports only compression types (RLE or no compression at all).
     */
    int *compression_types;

    /* The length of compression_types. */
    int compression_types_length;

    /* Preferred compression typed used by default. */
    int preferred_compression_type;

    /*
     * Minimum compression value. For lossy codecs more compression means less quality and vice versa.
     * For loseless codecs more compression means nothing else but a smaller file size. This field is
     * plugin-specific. If compression_min == compression_max == 0, no compression tuning is available.
     * For example: 0.
     */
    int compression_min;

    /*
     * Maximum compression value. This field is plugin-specific. If compression_min == compression_max == 0,
     * no compression tuning is available. For example: 100.
     */
    int compression_max;

    /* Default compression value. For example: 15. */
    int compression_default;
};

typedef struct sail_write_features sail_write_features_t;

/* Options to modify reading operations. */
struct sail_read_options {

    /*
     * Request to modify the output pixel format. Plugin (or an underlying codec) may reject
     * the requested pixel format.
     *
     * NOTE: Some input pixel formats might not map to some output pixel formats.
     *       SAIL returns an error in this case.
     */
    int pixel_format;

    /* IO manipulation options. See SailIoOptions. */
    int io_options;
};

typedef struct sail_read_options sail_read_options_t;

/* Options to modify writing operations. */
struct sail_write_options {

    /*
     * Request to modify the output pixel format. Plugin (or an underlying codec) may reject
     * the requested pixel format and return an error.
     *
     * NOTE: Some input pixel formats might not map to some output pixel formats.
     *       SAIL returns an error in this case.
     */
    int pixel_format;

    /* IO manipulation options. See SailIoOptions. */
    int io_options;

    /*
     * Compression type or 0. For example: SAIL_COMPRESSION_RLE. See SailCompression.
     * In most cases plugins mutually exclusive support either compression levels or compression types.
     * Use sail_write_features to determine what compression types or values are supported by a particular
     * plugin.
     *
     * For example:
     *
     *     1. The JPEG plugin supports only compression levels (compression_min, compression_max, compression_default).
     *     2. The TIFF plugin supports only compression types (RLE or no compression at all).
     */
    int compression_type;

    /*
     * Requested compression value. Must be in the range specified by compression_min and compression_max
     * in sail_write_features. If compression < compression_min, compression_default will be used.
     */
    int compression;
};

typedef struct sail_write_options sail_write_options_t;

/*
 * Image functions.
 */

/*
 * Allocates a new image. The assigned image MUST be destroyed later with sail_destroy_image().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_image(struct sail_image **image);

/*
 * Destroys the specified image and all its internal allocated memory buffers. The image MUST NOT be used anymore
 * after calling this function. Does nothing if the image is NULL.
 */
SAIL_EXPORT void sail_destroy_image(struct sail_image *image);

/*
 * Options functions.
 */

/*
 * Allocates read features. The assigned read features MUST be destroyed later
 * with sail_destroy_read_features().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_read_features(struct sail_read_features **read_features);

/*
 * Destroys the specified read features and all its internal allocated memory buffers. The read features
 * MUST NOT be used anymore after calling this function. Does nothing if the read features is NULL.
 */
SAIL_EXPORT void sail_destroy_read_features(struct sail_read_features *read_features);

/*
 * Allocates read options. The assigned read options MUST be destroyed later
 * with sail_destroy_read_options().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_read_options(struct sail_read_options **read_options);

/*
 * Destroys the specified read options and all its internal allocated memory buffers. The read options
 * MUST NOT be used anymore after calling this function. Does nothing if the read options is NULL.
 */
SAIL_EXPORT void sail_destroy_read_options(struct sail_read_options *read_options);

/*
 * Allocates write features. The assigned write features MUST be destroyed later
 * with sail_destroy_write_features().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_write_features(struct sail_write_features **write_features);

/*
 * Destroys the specified write features and all its internal allocated memory buffers. The write features
 * MUST NOT be used anymore after calling this function. Does nothing if the write features is NULL.
 */
SAIL_EXPORT void sail_destroy_write_features(struct sail_write_features *write_features);

/*
 * Allocates write options. The assigned write options MUST be destroyed later
 * with sail_destroy_write_options().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_write_options(struct sail_write_options **write_options);

/*
 * Destroys the specified write options and all its internal allocated memory buffers. The write options
 * MUST NOT be used anymore after calling this function. Does nothing if the write options is NULL.
 */
SAIL_EXPORT void sail_destroy_write_options(struct sail_write_options *write_options);

/*
 * Builds default read options from read features.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_read_options_from_features(const struct sail_read_features *read_features, struct sail_read_options *read_options);

/*
 * Allocates and builds default read options from read features. The assigned read options MUST be destroyed later
 * with sail_destroy_read_options().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_read_options_from_features(const struct sail_read_features *read_features, struct sail_read_options **read_options);

/*
 * Builds default write options from write features.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_write_options_from_features(const struct sail_write_features *write_features, struct sail_write_options *write_options);

/*
 * Allocates and builds default write options from write features. The assigned write options MUST be destroyed later
 * with sail_destroy_write_options().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_write_options_from_features(const struct sail_write_features *write_features, struct sail_write_options **write_options);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
