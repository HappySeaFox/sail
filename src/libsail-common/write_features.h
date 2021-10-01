/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

    The MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef SAIL_WRITE_FEATURES_H
#define SAIL_WRITE_FEATURES_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_pixel_formats_mapping_node;

/*
 * Write features. Use this structure to determine what a codec can actually write.
 */
struct sail_write_features {

    /*
     * A list of supported pixel formats that can be written by this codec.
     */
    enum SailPixelFormat *output_pixel_formats;

    /* The length of output_pixel_formats. */
    unsigned output_pixel_formats_length;

    /* Supported or-ed features of writing operations. See SailCodecFeature. */
    int features;

    /*
     * Required or-ed image properties. For example, an input image must be flipped by a caller
     * before writing it with SAIL. See SailImageProperty.
     */
    int properties;

    /*
     * A list of supported pixels compression types by this codec. If the list has more than
     * two entries, compression levels are ignored.
     *
     * For example:
     *
     *     1. The JPEG codec supports only one compression, JPEG. compression_level_min, compression_level_max,
     *        compression_level_default can be used to select a compression level.
     *     2. The TIFF codec supports more than two compression types (PACKBITS, JPEG, etc.).
     *        Compression levels are ignored.
     */
    enum SailCompression *compressions;

    /* The length of compressions. */
    unsigned compressions_length;

    /* Compression type to use by default. */
    enum SailCompression default_compression;

    /*
     * Minimum compression value. For lossy codecs, more compression means less quality and vice versa.
     * For lossless codecs, more compression means nothing but a smaller file size. This field is
     * codec-specific. If compression_level_min == compression_level_max == 0, no compression tuning is available.
     * For example: 0.
     */
    double compression_level_min;

    /*
     * Maximum compression value. This field is codec-specific. If compression_level_min == compression_level_max == 0,
     * no compression tuning is available. For example: 100.
     */
    double compression_level_max;

    /* Default compression value. For example: 15. */
    double compression_level_default;

    /* Step to increase or decrease compression levels. For example: 1. */
    double compression_level_step;
};

typedef struct sail_write_features sail_write_features_t;

/*
 * Allocates write features. The assigned write features MUST be destroyed later
 * with sail_destroy_write_features().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_write_features(struct sail_write_features **write_features);

/*
 * Destroys the specified write features object and all its internal allocated memory buffers.
 * The write features MUST NOT be used after calling this function. It does nothing if the write features is NULL.
 */
SAIL_EXPORT void sail_destroy_write_features(struct sail_write_features *write_features);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
