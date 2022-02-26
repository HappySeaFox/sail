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

static sail_status_t save_str_in_meta_data(const char *str, unsigned length_wo_null, enum SailMetaData key, struct sail_meta_data_node **meta_data_node) {

    SAIL_CHECK_PTR(str);
    SAIL_CHECK_PTR(meta_data_node);

    struct sail_meta_data_node *meta_data_node_local;

    SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node_local));

    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_from_known_key(key, &meta_data_node_local->meta_data),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&meta_data_node_local->meta_data->value),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_substring(meta_data_node_local->meta_data->value, str, length_wo_null),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));

    *meta_data_node = meta_data_node_local;

    return SAIL_OK;
}

sail_status_t gif_private_fetch_comment(const GifByteType *extension, struct sail_meta_data_node **meta_data_node) {

    SAIL_CHECK_PTR(extension);

    const unsigned length = *(unsigned char *)(extension);

    if (length == 0) {
        return SAIL_OK;
    }

    SAIL_TRY(save_str_in_meta_data((const char *)extension + 1, length, SAIL_META_DATA_COMMENT, meta_data_node));

    return SAIL_OK;
}

sail_status_t gif_private_fetch_application(const GifByteType *extension, struct sail_meta_data_node **meta_data_node) {

    SAIL_CHECK_PTR(extension);

    /* 8 bytes as per the spec. */
    SAIL_TRY(save_str_in_meta_data((const char *)extension + 1, 8, SAIL_META_DATA_SOFTWARE, meta_data_node));

    return SAIL_OK;
}
