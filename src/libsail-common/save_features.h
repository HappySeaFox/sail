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

#ifndef SAIL_SAVE_FEATURES_H
#define SAIL_SAVE_FEATURES_H

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

struct sail_string_node;

/*
 * Save features. Use this structure to determine what a codec can actually save.
 */
struct sail_save_features {

    /*
     * A list of supported pixel formats that can be written by this codec.
     */
    enum SailPixelFormat *pixel_formats;

    /* The length of pixel_formats. */
    unsigned pixel_formats_length;

    /* Supported or-ed features of saving operations. See SailCodecFeature. */
    int features;

    /*
     * A list of supported pixels compression types by this codec. If the list has more than
     * two entries, compression levels are ignored.
     *
     * For example:
     *
     *     1. The JPEG codec supports only one compression, JPEG. compression_level can be used
     *        to select a compression level.
     *     2. The TIFF codec supports more than two compression types (PACKBITS, JPEG, etc.).
     *        Compression levels are ignored.
     */
    enum SailCompression *compressions;

    /* The length of compressions. */
    unsigned compressions_length;

    /* Compression type to use by default. */
    enum SailCompression default_compression;

    /*
     * Supported compression level range. If compression levels are not supported
     * by the codec, compression_level is NULL.
     */
    struct sail_compression_level *compression_level;

    /*
     * Codec-specific tuning options. For example, a hypothetical ABC image codec
     * can allow disabling filtering with setting the "abc-filtering" tuning option
     * to 0 in load options. Tuning options' names start with the codec name
     * to avoid confusing.
     *
     * The list of possible values for every tuning option is not current available
     * programmatically. Every codec must document them in the codec info.
     *
     * It's not guaranteed that tuning options and their values are backward
     * or forward compatible.
     */
    struct sail_string_node *tuning;
};

typedef struct sail_save_features sail_save_features_t;

/*
 * Allocates save features.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_save_features(struct sail_save_features **save_features);

/*
 * Destroys the specified save features object and all its internal allocated memory buffers.
 * The save features MUST NOT be used after calling this function. It does nothing if the save features is NULL.
 */
SAIL_EXPORT void sail_destroy_save_features(struct sail_save_features *save_features);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
