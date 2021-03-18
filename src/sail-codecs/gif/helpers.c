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

#include <string.h>

#include <gif_lib.h>

#include "sail-common.h"

#include "helpers.h"

sail_status_t gif_private_supported_read_output_pixel_format(enum SailPixelFormat pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        case SAIL_PIXEL_FORMAT_BPP32_BGRA: {
            return SAIL_OK;
        }

        default: {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
    }
}

static sail_status_t save_str_in_meta_data(const char *str, unsigned length_wo_null, enum SailMetaData key, struct sail_meta_data_node **image_meta_data_node) {

    SAIL_CHECK_STRING_PTR(str);

    /* Allocate a new meta data entry. */
    struct sail_meta_data_node *meta_data_node;

    SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node));

    meta_data_node->key = key;
    meta_data_node->value_type = SAIL_META_DATA_TYPE_STRING;
    meta_data_node->value_length = length_wo_null + 1;

    SAIL_TRY_OR_CLEANUP(sail_malloc(meta_data_node->value_length, &meta_data_node->value),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node));

    memcpy(meta_data_node->value, str, meta_data_node->value_length - 1);
    *((char *)meta_data_node->value + meta_data_node->value_length - 1) = '\0';

    /* Save it as a last meta data node in the image. */
    while (*image_meta_data_node != NULL) {
        *image_meta_data_node = (*image_meta_data_node)->next;
    }

    *image_meta_data_node = meta_data_node;

    return SAIL_OK;
}

sail_status_t gif_private_fetch_comment(const GifByteType *extension, struct sail_meta_data_node **image_meta_data_node) {

    SAIL_CHECK_PTR(extension);

    const int length = extension[0];

    if (length <= 0) {
        return SAIL_OK;
    }

    SAIL_TRY(save_str_in_meta_data((const char *)extension + 1, length, SAIL_META_DATA_COMMENT, image_meta_data_node));

    return SAIL_OK;
}

sail_status_t gif_private_fetch_application(const GifByteType *extension, struct sail_meta_data_node **image_meta_data_node) {

    SAIL_CHECK_PTR(extension);

    /* 8 bytes as per the spec. */
    SAIL_TRY(save_str_in_meta_data((const char *)extension + 1, 8, SAIL_META_DATA_SOFTWARE, image_meta_data_node));

    return SAIL_OK;
}
