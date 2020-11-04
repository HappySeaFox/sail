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

#include <gif_lib.h>

#include "sail-common.h"

#include "helpers.h"

sail_status_t supported_read_output_pixel_format(enum SailPixelFormat pixel_format) {

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

sail_status_t fetch_comment(const GifByteType *extension, struct sail_meta_data_node **image_meta_data_node) {

    SAIL_CHECK_PTR(extension);

    const int length = extension[0];

    if (length <= 0) {
        return SAIL_OK;
    }

    /* Allocate a new meta data entry. */
    struct sail_meta_data_node *meta_data_node;

    SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node));

    meta_data_node->key = SAIL_META_DATA_COMMENT;
    meta_data_node->value_type = SAIL_META_DATA_TYPE_STRING;

    SAIL_TRY_OR_CLEANUP(sail_strdup_length((const char *)(extension + 1), length, &meta_data_node->value_string),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node));

    /* Save it as a last meta data node in the image. */
    while (*image_meta_data_node != NULL) {
        *image_meta_data_node = (*image_meta_data_node)->next;
    }

    *image_meta_data_node = meta_data_node;

    return SAIL_OK;
}

sail_status_t fetch_application(const GifByteType *extension, struct sail_meta_data_node **image_meta_data_node) {

    SAIL_CHECK_PTR(extension);

    /* Allocate a new meta data entry. */
    struct sail_meta_data_node *meta_data_node;

    SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node));

    meta_data_node->key = SAIL_META_DATA_SOFTWARE;
    meta_data_node->value_type = SAIL_META_DATA_TYPE_STRING;

    /* 8 bytes as per the spec. */
    SAIL_TRY_OR_CLEANUP(sail_strdup_length((const char *)(extension + 1), 8, &meta_data_node->value_string),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node));

    /* Save it as a last meta data node in the image. */
    while (*image_meta_data_node != NULL) {
        *image_meta_data_node = (*image_meta_data_node)->next;
    }

    *image_meta_data_node = meta_data_node;

    return SAIL_OK;
}
