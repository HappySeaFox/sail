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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <libheif/heif.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

struct heif_context;
struct heif_encoder;
struct heif_image;
struct heif_image_handle;
struct sail_hash_map;
struct sail_iccp;
struct sail_meta_data_node;
struct sail_variant;

/* Tuning state structure for passing parameters. */
struct heif_tuning_state
{
    struct heif_encoder* encoder;
    int* threads;
};

SAIL_HIDDEN enum SailPixelFormat heif_private_sail_pixel_format_from_heif(enum heif_chroma chroma,
                                                                          enum heif_channel channel,
                                                                          int bits_per_pixel);

SAIL_HIDDEN bool heif_private_heif_chroma_from_sail_pixel_format(enum SailPixelFormat pixel_format,
                                                                 enum heif_chroma* chroma,
                                                                 int* bits_per_component,
                                                                 bool* has_alpha);

SAIL_HIDDEN sail_status_t heif_private_fetch_iccp(struct heif_image_handle* handle, struct sail_iccp** iccp);

SAIL_HIDDEN sail_status_t heif_private_fetch_meta_data(struct heif_image_handle* handle,
                                                       struct sail_meta_data_node** meta_data_node);

SAIL_HIDDEN sail_status_t heif_private_write_iccp(struct heif_image* image, const struct sail_iccp* iccp);

SAIL_HIDDEN sail_status_t heif_private_write_meta_data(struct heif_context* ctx,
                                                       struct heif_image_handle* handle,
                                                       const struct sail_meta_data_node* meta_data_node);

/* Specialized properties functions. */
SAIL_HIDDEN sail_status_t heif_private_fetch_depth_info(const struct heif_image_handle* image_handle,
                                                        struct sail_hash_map* special_properties);

SAIL_HIDDEN sail_status_t heif_private_fetch_thumbnail_info(const struct heif_image_handle* image_handle,
                                                            struct sail_hash_map* special_properties);

SAIL_HIDDEN sail_status_t heif_private_fetch_primary_flag(const struct heif_image_handle* image_handle,
                                                          struct sail_hash_map* special_properties);

SAIL_HIDDEN sail_status_t heif_private_fetch_hdr_metadata(const struct heif_image* heif_image,
                                                          struct sail_hash_map* special_properties);

SAIL_HIDDEN sail_status_t heif_private_fetch_premultiplied_alpha(const struct heif_image* heif_image,
                                                                 struct sail_hash_map* special_properties);

/* Tuning callback. */
SAIL_HIDDEN bool heif_private_tuning_key_value_callback(const char* key,
                                                        const struct sail_variant* value,
                                                        void* user_data);

/* Convert heif_error to sail_status_t. */
SAIL_HIDDEN sail_status_t heif_private_heif_error_to_sail_status(const struct heif_error* error);
