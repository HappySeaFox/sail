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

#ifndef SAIL_META_DATA_H
#define SAIL_META_DATA_H

#include <stddef.h>

#ifdef SAIL_BUILD
    #include "common.h"
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/common.h>
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_variant;

/*
 * Represents a meta data element like a JPEG comment or a binary EXIF profile.
 *
 * For example:
 *
 * {
 *     key         = SAIL_META_DATA_UNKNOWN,
 *     key_unknown = "My Data",
 *     value       = { SAIL_VARIANT_TYPE_STRING, "Data" } 
 * }
 *
 * {
 *     key         = SAIL_META_DATA_COMMENT,
 *     key_unknown = NULL,
 *     value       = { SAIL_VARIANT_TYPE_STRING, "Holidays" }
 * }
 *
 * {
 *     key         = SAIL_META_DATA_EXIF,
 *     key_unknown = NULL,
 *     value       = { SAIL_VARIANT_TYPE_DATA, <binary data> }
 * }
 *
 * Not every image codec supports key-values. For example:
 *
 *   - JPEG doesn't support keys. When you try to save an image with meta data,
 *     only values are saved.
 *   - TIFF supports only a subset of known meta data keys (Artist, Make, Model etc.).
 *     It doesn't support saving unknown keys (SAIL_META_DATA_UNKNOWN).
 *   - PNG supports both keys and values.
 *
 * When saving images, SAIL codecs don't necessarily use sail_meta_data_to_string() to convert
 * keys to string representations. PNG, for example, uses hardcoded "Raw profile type exif" key name
 * for EXIF tags.
 */
struct sail_meta_data {

    /*
     * If the key is SAIL_META_DATA_UNKNOWN, key_unknown contains an actual string key.
     * If the key is other than SAIL_META_DATA_UNKNOWN, key_unknown is NULL.
     */
    enum SailMetaData key;
    char *key_unknown;

    /*
     * Meta data value.
     */
    struct sail_variant *value;
};

/*
 * Allocates new meta data.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_meta_data(struct sail_meta_data **meta_data);

/*
 * Allocates new meta data from the specified known key.
 * The key must not be SAIL_META_DATA_UNKNOWN.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_meta_data_from_known_key(enum SailMetaData key,
                                                              struct sail_meta_data **meta_data);

/*
 * Allocates new meta data from the specified unknown key. Makes a deep copy of the key.
 * Sets the key to SAIL_META_DATA_UNKNOWN.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_meta_data_from_unknown_key(const char *key_unknown,
                                                                struct sail_meta_data **meta_data);

/*
 * Destroys the specified meta data.
 */
SAIL_EXPORT void sail_destroy_meta_data(struct sail_meta_data *meta_data);

/*
 * Makes a deep copy of the specified meta data.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_meta_data(const struct sail_meta_data *source,
                                              struct sail_meta_data **target);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
