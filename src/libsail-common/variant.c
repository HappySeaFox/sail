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
static sail_status_t set_variant_value(struct sail_variant *variant, enum SailVariantType type, const void *value, const size_t size) {

    SAIL_CHECK_PTR(variant);

    void **ptr = &variant->value;
    SAIL_TRY(sail_realloc(size, ptr));
    memcpy(variant->value, value, size);

    variant->type = type;
    variant->size = size;

    return SAIL_OK;
}

static sail_status_t alloc_variant(enum SailVariantType type, const void *value, const size_t size, struct sail_variant **variant) {

    SAIL_CHECK_PTR(variant);

    struct sail_variant *variant_local;
    SAIL_TRY(sail_alloc_variant(&variant_local));

    SAIL_TRY_OR_CLEANUP(set_variant_value(variant_local, type, value, size),
                        /* on error */ sail_destroy_variant(variant_local));

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

    (*variant)->type  = SAIL_VARIANT_TYPE_INVALID;
    (*variant)->value = NULL;
    (*variant)->size  = 0;

    return SAIL_OK;
}

void sail_destroy_variant(struct sail_variant *variant) {

    if (variant == NULL) {
        return;
    }

    sail_free(variant->value);
    sail_free(variant);
}

sail_status_t sail_set_variant_bool(struct sail_variant *variant, bool value) {

    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_BOOL, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_char(struct sail_variant *variant, char value) {

    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_CHAR, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_unsigned_char(struct sail_variant *variant, unsigned char value) {

    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_UNSIGNED_CHAR, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_short(struct sail_variant *variant, short value) {

    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_SHORT, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_unsigned_short(struct sail_variant *variant, unsigned short value) {

    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_UNSIGNED_SHORT, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_int(struct sail_variant *variant, int value) {

    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_INT, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_unsigned_int(struct sail_variant *variant, unsigned int value) {

    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_UNSIGNED_INT, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_long(struct sail_variant *variant, long value) {

    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_LONG, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_unsigned_long(struct sail_variant *variant, unsigned long value) {

    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_UNSIGNED_LONG, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_float(struct sail_variant *variant, float value) {

    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_FLOAT, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_double(struct sail_variant *variant, double value) {

    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_DOUBLE, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_string(struct sail_variant *variant, const char *value) {

    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_STRING, value, strlen(value) + 1));

    return SAIL_OK;
}

sail_status_t sail_set_variant_adopted_string(struct sail_variant *variant, char *value) {

    SAIL_CHECK_PTR(variant);

    sail_free(variant->value);

    variant->type  = SAIL_VARIANT_TYPE_STRING;
    variant->value = value;
    variant->size  = strlen(value) + 1;

    return SAIL_OK;
}

sail_status_t sail_set_variant_substring(struct sail_variant *variant, const char *value, size_t size) {

    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_STRING, value, size + 1));

    char *str = variant->value;
    str[size] = '\0';

    return SAIL_OK;
}

sail_status_t sail_set_variant_data(struct sail_variant *variant, const void *value, size_t size) {

    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_DATA, value, size));

    return SAIL_OK;
}

sail_status_t sail_set_variant_adopted_data(struct sail_variant *variant, void *value, size_t size) {

    SAIL_CHECK_PTR(variant);

    sail_free(variant->value);

    variant->type  = SAIL_VARIANT_TYPE_DATA;
    variant->value = value;
    variant->size  = size;

    return SAIL_OK;
}

bool sail_variant_to_bool(const struct sail_variant *variant)
{
    return *(bool *)(variant->value);
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

float sail_variant_to_float(const struct sail_variant *variant)
{
    return *(float *)(variant->value);
}

double sail_variant_to_double(const struct sail_variant *variant)
{
    return *(double *)(variant->value);
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

    if (variant->type == SAIL_VARIANT_TYPE_INVALID || variant->value == NULL || variant->size == 0) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_VARIANT);
    }

    return SAIL_OK;
}

sail_status_t sail_copy_variant(const struct sail_variant *source, struct sail_variant **target) {

    SAIL_CHECK_PTR(source);
    SAIL_CHECK_PTR(target);

    SAIL_TRY(alloc_variant(source->type, source->value, source->size, target));

    return SAIL_OK;
}

bool sail_equal_variants(const struct sail_variant *variant1, const struct sail_variant *variant2) {

    SAIL_TRY_OR_EXECUTE(sail_check_variant_valid(variant1),
                        /* on error */ return false);
    SAIL_TRY_OR_EXECUTE(sail_check_variant_valid(variant2),
                        /* on error */ return false);

    if (variant1->type != variant2->type || variant1->size != variant2->size) {
        return false;
    } else {
        return memcmp(variant1->value, variant2->value, variant1->size) == 0;
    }
}
