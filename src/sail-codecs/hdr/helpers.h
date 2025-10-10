/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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

#ifndef SAIL_HDR_HELPERS_H
#define SAIL_HDR_HELPERS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

struct sail_io;
struct sail_hash_map;
struct sail_meta_data_node;
struct sail_variant;

/* HDR file header information. */
struct hdr_header {
    int width;
    int height;
    bool y_increasing;
    bool x_increasing;
    float exposure;
    float gamma;
    char *software;
    char *view;
    char *primaries;
    float colorcorr[3];
};

/* Tuning context for encoding. */
struct hdr_write_context {
    bool use_rle;
    struct hdr_header *header;
};

SAIL_HIDDEN bool hdr_private_is_hdr(const void *data, size_t size);

SAIL_HIDDEN sail_status_t hdr_private_read_header(struct sail_io *io, struct hdr_header *header);

SAIL_HIDDEN sail_status_t hdr_private_write_header(struct sail_io *io, const struct hdr_header *header,
                                                      const struct sail_meta_data_node *meta_data_node);

SAIL_HIDDEN sail_status_t hdr_private_read_scanline(struct sail_io *io, int width, float *scanline);

SAIL_HIDDEN sail_status_t hdr_private_write_scanline(struct sail_io *io, int width, const float *scanline, bool use_rle);

SAIL_HIDDEN void hdr_private_rgbe_to_float(const uint8_t *rgbe, float *rgb);

SAIL_HIDDEN void hdr_private_float_to_rgbe(const float *rgb, uint8_t *rgbe);

SAIL_HIDDEN void hdr_private_destroy_header(struct hdr_header *header);

SAIL_HIDDEN sail_status_t hdr_private_store_properties(const struct hdr_header *header, struct sail_hash_map *special_properties);

SAIL_HIDDEN sail_status_t hdr_private_fetch_properties(const struct sail_hash_map *special_properties, struct hdr_header *header);

SAIL_HIDDEN bool hdr_private_tuning_key_value_callback(const char *key, const struct sail_variant *value, void *user_data);

#endif
