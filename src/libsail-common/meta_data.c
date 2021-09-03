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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

sail_status_t sail_alloc_meta_data(struct sail_meta_data **meta_data) {

    SAIL_CHECK_PTR(meta_data);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_meta_data), &ptr));
    *meta_data = ptr;

    (*meta_data)->key          = SAIL_META_DATA_UNKNOWN;
    (*meta_data)->key_unknown  = NULL;
    (*meta_data)->value_type   = SAIL_META_DATA_TYPE_STRING;
    (*meta_data)->value        = NULL;
    (*meta_data)->value_length = 0;

    return SAIL_OK;
}

sail_status_t sail_alloc_meta_data_from_known_string(enum SailMetaData key, const char *value, struct sail_meta_data **meta_data) {

    SAIL_CHECK_PTR(value);

    SAIL_TRY(sail_alloc_meta_data_from_known_substring(key, value, strlen(value), meta_data));

    return SAIL_OK;
}

sail_status_t sail_alloc_meta_data_from_unknown_string(const char *key_unknown, const char *value, struct sail_meta_data **meta_data) {

    SAIL_CHECK_PTR(value);

    SAIL_TRY(sail_alloc_meta_data_from_unknown_substring(key_unknown, value, strlen(value), meta_data));

    return SAIL_OK;
}

sail_status_t sail_alloc_meta_data_from_known_substring(enum SailMetaData key, const char *value, size_t size, struct sail_meta_data **meta_data) {

    if (key == SAIL_META_DATA_UNKNOWN) {
        SAIL_LOG_ERROR("%s() accepts only known meta data keys", __func__);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_CHECK_PTR(value);
    SAIL_CHECK_PTR(meta_data);

    struct sail_meta_data *meta_data_local;
    SAIL_TRY(sail_alloc_meta_data(&meta_data_local));

    meta_data_local->key          = key;
    meta_data_local->value_type   = SAIL_META_DATA_TYPE_STRING;
    meta_data_local->value_length = size + 1;

    SAIL_TRY_OR_CLEANUP(sail_malloc(meta_data_local->value_length, &meta_data_local->value),
                        /* cleanup */ sail_destroy_meta_data(meta_data_local));

    memcpy(meta_data_local->value, value, meta_data_local->value_length - 1);
    *((char *)meta_data_local->value + meta_data_local->value_length - 1) = '\0';

    *meta_data = meta_data_local;

    return SAIL_OK;
}

sail_status_t sail_alloc_meta_data_from_unknown_substring(const char *key_unknown, const char *value, size_t size, struct sail_meta_data **meta_data) {

    SAIL_CHECK_PTR(key_unknown);
    SAIL_CHECK_PTR(value);
    SAIL_CHECK_PTR(meta_data);

    struct sail_meta_data *meta_data_local;
    SAIL_TRY(sail_alloc_meta_data(&meta_data_local));

    SAIL_TRY_OR_CLEANUP(sail_strdup(key_unknown, &meta_data_local->key_unknown),
                        /* cleanup */ sail_destroy_meta_data(meta_data_local));

    meta_data_local->key          = SAIL_META_DATA_UNKNOWN;
    meta_data_local->value_type   = SAIL_META_DATA_TYPE_STRING;
    meta_data_local->value_length = size + 1;

    SAIL_TRY_OR_CLEANUP(sail_malloc(meta_data_local->value_length, &meta_data_local->value),
                        /* cleanup */ sail_destroy_meta_data(meta_data_local));

    memcpy(meta_data_local->value, value, meta_data_local->value_length - 1);
    *((char *)meta_data_local->value + meta_data_local->value_length - 1) = '\0';

    *meta_data = meta_data_local;

    return SAIL_OK;
}

sail_status_t sail_alloc_meta_data_from_known_data(enum SailMetaData key, const void *value, size_t value_length, struct sail_meta_data **meta_data) {

    if (key == SAIL_META_DATA_UNKNOWN) {
        SAIL_LOG_ERROR("%s() accepts only known meta data keys", __func__);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_CHECK_PTR(value);
    SAIL_CHECK_PTR(meta_data);

    struct sail_meta_data *meta_data_local;
    SAIL_TRY(sail_alloc_meta_data(&meta_data_local));

    meta_data_local->key          = key;
    meta_data_local->value_type   = SAIL_META_DATA_TYPE_DATA;
    meta_data_local->value_length = value_length;

    SAIL_TRY_OR_CLEANUP(sail_memdup(value, value_length, &meta_data_local->value),
                        /* cleanup */ sail_destroy_meta_data(meta_data_local));

    *meta_data = meta_data_local;

    return SAIL_OK;
}

sail_status_t sail_alloc_meta_data_from_unknown_data(const char *key_unknown, const void *value, size_t value_length, struct sail_meta_data **meta_data) {

    SAIL_CHECK_PTR(key_unknown);
    SAIL_CHECK_PTR(value);
    SAIL_CHECK_PTR(meta_data);

    struct sail_meta_data *meta_data_local;
    SAIL_TRY(sail_alloc_meta_data(&meta_data_local));

    SAIL_TRY_OR_CLEANUP(sail_strdup(key_unknown, &meta_data_local->key_unknown),
                        /* cleanup */ sail_destroy_meta_data(meta_data_local));

    meta_data_local->key          = SAIL_META_DATA_UNKNOWN;
    meta_data_local->value_type   = SAIL_META_DATA_TYPE_DATA;
    meta_data_local->value_length = value_length;

    SAIL_TRY_OR_CLEANUP(sail_memdup(value, value_length, &meta_data_local->value),
                        /* cleanup */ sail_destroy_meta_data(meta_data_local));

    *meta_data = meta_data_local;

    return SAIL_OK;
}

void sail_destroy_meta_data(struct sail_meta_data *meta_data) {

    if (meta_data == NULL) {
        return;
    }

    sail_free(meta_data->key_unknown);
    sail_free(meta_data->value);
    sail_free(meta_data);
}

sail_status_t sail_copy_meta_data(const struct sail_meta_data *source, struct sail_meta_data **target) {

    SAIL_CHECK_PTR(source);
    SAIL_CHECK_PTR(target);

    struct sail_meta_data *meta_data_local;
    SAIL_TRY(sail_alloc_meta_data(&meta_data_local));

    meta_data_local->key = source->key;

    if (source->key_unknown != NULL) {
        SAIL_TRY_OR_CLEANUP(sail_strdup(source->key_unknown, &meta_data_local->key_unknown),
                            /* cleanup */ sail_destroy_meta_data(meta_data_local));
    }

    meta_data_local->value_type = source->value_type;

    SAIL_TRY_OR_CLEANUP(sail_memdup(source->value, source->value_length, &meta_data_local->value),
                        /* cleanup */ sail_destroy_meta_data(meta_data_local));

    meta_data_local->value_length = source->value_length;

    *target = meta_data_local;

    return SAIL_OK;
}
