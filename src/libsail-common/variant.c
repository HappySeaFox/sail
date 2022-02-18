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

#include "sail-common.h"

/*
 * Private functions.
 */
static sail_status_t alloc_variant(enum SailVariantType value_type, void *value, size_t value_size, struct sail_variant **variant) {

    SAIL_CHECK_PTR(variant);

    struct sail_variant *variant_local;
    SAIL_TRY(sail_alloc_variant(&variant_local));

    void *ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(value_size, &ptr),
                        /* on error */ sail_destroy_variant(variant_local));
    variant_local->value = ptr;
    memcpy(variant_local->value, value, value_size);

    variant_local->value_type = value_type;
    variant_local->value_size = value_size;

    *variant = variant_local;

    return SAIL_OK;
}

/*
 * Public functions.
 */
sail_status_t sail_alloc_variant(struct sail_variant **variant) {

    SAIL_CHECK_PTR(variant);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_variant), &ptr));
    *variant = ptr;

    (*variant)->value_type  = SAIL_META_DATA_UNKNOWN;
    (*variant)->value       = NULL;
    (*variant)->value_size  = SAIL_VARIANT_TYPE_INVALID;

    return SAIL_OK;
}

sail_status_t sail_alloc_variant_from_char(char value, struct sail_variant **variant)
{
    SAIL_TRY(alloc_variant(SAIL_VARIANT_TYPE_CHAR, &value, sizeof(value), variant));

    return SAIL_OK;
}

sail_status_t sail_alloc_variant_from_unsigned_char(unsigned char value, struct sail_variant **variant)
{
    SAIL_TRY(alloc_variant(SAIL_VARIANT_TYPE_UNSIGNED_CHAR, &value, sizeof(value), variant));

    return SAIL_OK;
}

sail_status_t sail_alloc_variant_from_short(short value, struct sail_variant **variant)
{
    SAIL_TRY(alloc_variant(SAIL_VARIANT_TYPE_SHORT, &value, sizeof(value), variant));

    return SAIL_OK;
}

sail_status_t sail_alloc_variant_from_unsigned_short(unsigned short value, struct sail_variant **variant)
{
    SAIL_TRY(alloc_variant(SAIL_VARIANT_TYPE_UNSIGNED_SHORT, &value, sizeof(value), variant));

    return SAIL_OK;
}

sail_status_t sail_alloc_variant_from_int(int value, struct sail_variant **variant)
{
    SAIL_TRY(alloc_variant(SAIL_VARIANT_TYPE_INT, &value, sizeof(value), variant));

    return SAIL_OK;
}

sail_status_t sail_alloc_variant_from_unsigned_int(unsigned int value, struct sail_variant **variant)
{
    SAIL_TRY(alloc_variant(SAIL_VARIANT_TYPE_UNSIGNED_INT, &value, sizeof(value), variant));

    return SAIL_OK;
}

sail_status_t sail_alloc_variant_from_long(long value, struct sail_variant **variant)
{
    SAIL_TRY(alloc_variant(SAIL_VARIANT_TYPE_LONG, &value, sizeof(value), variant));

    return SAIL_OK;
}

sail_status_t sail_alloc_variant_from_unsigned_long(unsigned long value, struct sail_variant **variant)
{
    SAIL_TRY(alloc_variant(SAIL_VARIANT_TYPE_UNSIGNED_LONG, &value, sizeof(value), variant));

    return SAIL_OK;
}

sail_status_t sail_alloc_variant_from_data(void *value, size_t value_size, struct sail_variant **variant)
{
    SAIL_TRY(alloc_variant(SAIL_VARIANT_TYPE_DATA, value, value_size, variant));

    return SAIL_OK;
}

sail_status_t sail_alloc_variant_from_stolen_data(void *value, size_t value_size, struct sail_variant **variant)
{
    SAIL_TRY(sail_alloc_variant(variant));

    (*variant)->value_type = SAIL_VARIANT_TYPE_DATA;
    (*variant)->value      = value;
    (*variant)->value_size = value_size;

    return SAIL_OK;
}

sail_status_t sail_alloc_variant_from_string(char *value, struct sail_variant **variant)
{
    SAIL_TRY(alloc_variant(SAIL_VARIANT_TYPE_STRING, value, strlen(value) + 1, variant));

    return SAIL_OK;
}

sail_status_t sail_alloc_variant_from_stolen_string(char *value, struct sail_variant **variant)
{
    SAIL_TRY(sail_alloc_variant(variant));

    (*variant)->value_type = SAIL_VARIANT_TYPE_STRING;
    (*variant)->value      = value;
    (*variant)->value_size = strlen(value) + 1;

    return SAIL_OK;
}

sail_status_t sail_alloc_variant_from_substring(char *value, size_t value_size, struct sail_variant **variant)
{
    SAIL_TRY(alloc_variant(SAIL_VARIANT_TYPE_STRING, value, value_size + 1, variant));

    char *str = (*variant)->value;
    str[value_size] = '\0';

    return SAIL_OK;
}

void sail_destroy_variant(struct sail_variant *variant)
{
    if (variant == NULL) {
        return;
    }

    sail_free(variant->value);
    sail_free(variant);
}

char sail_variant_to_char(const struct sail_variant *variant)
{
    return *(char *)(variant->value);
}

unsigned char sail_variant_to_unsigned_char(const struct sail_variant *variant)
{
    return *(unsigned char *)(variant->value);
}

short sail_variant_to_short(const struct sail_variant *variant)
{
    return *(short *)(variant->value);
}

unsigned short sail_variant_to_unsigned_short(const struct sail_variant *variant)
{
    return *(unsigned short *)(variant->value);
}

int sail_variant_to_int(const struct sail_variant *variant)
{
    return *(int *)(variant->value);
}

unsigned int sail_variant_to_unsigned_int(const struct sail_variant *variant)
{
    return *(unsigned int *)(variant->value);
}

long sail_variant_to_long(const struct sail_variant *variant)
{
    return *(long *)(variant->value);
}

unsigned long sail_variant_to_unsigned_long(const struct sail_variant *variant)
{
    return *(unsigned long *)(variant->value);
}

char* sail_variant_to_string(const struct sail_variant *variant)
{
    return (char *)(variant->value);
}

void* sail_variant_to_data(const struct sail_variant *variant)
{
    return variant->value;
}

sail_status_t sail_check_variant_valid(const struct sail_variant *variant)
{
    SAIL_CHECK_PTR(variant);

    if (variant->value_type == SAIL_VARIANT_TYPE_INVALID || variant->value == NULL || variant->value_size == 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_VARIANT);
    }

    return SAIL_OK;
}

sail_status_t sail_copy_variant(const struct sail_variant *source, struct sail_variant **target) {

    SAIL_CHECK_PTR(source);
    SAIL_CHECK_PTR(target);

    SAIL_TRY(alloc_variant(source->value_type, source->value, source->value_size, target));

    return SAIL_OK;
}
