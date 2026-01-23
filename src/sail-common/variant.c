/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sail-common.h"

/*
 * Private functions.
 */
static sail_status_t set_variant_value_asymmetric(struct sail_variant* variant,
                                                  enum SailVariantType type,
                                                  const void* value,
                                                  const size_t variant_size,
                                                  const size_t value_size)
{
    SAIL_CHECK_PTR(variant);

    if (variant_size < value_size)
    {
        SAIL_LOG_ERROR("Variant size %zu is less than value size %zu", variant_size, value_size);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    /* Free old value before allocating new one. */
    sail_free(variant->value);
    variant->value = NULL;

    void** ptr = &variant->value;
    SAIL_TRY(sail_realloc(variant_size, ptr));
    memcpy(variant->value, value, value_size);

    variant->type = type;
    variant->size = variant_size;

    return SAIL_OK;
}

static sail_status_t set_variant_value(struct sail_variant* variant,
                                       enum SailVariantType type,
                                       const void* value,
                                       const size_t size)
{
    SAIL_TRY(set_variant_value_asymmetric(variant, type, value, size, size));

    return SAIL_OK;
}

static sail_status_t alloc_variant(enum SailVariantType type,
                                   const void* value,
                                   const size_t size,
                                   struct sail_variant** variant)
{
    SAIL_CHECK_PTR(variant);

    struct sail_variant* variant_local;
    SAIL_TRY(sail_alloc_variant(&variant_local));

    SAIL_TRY_OR_CLEANUP(set_variant_value(variant_local, type, value, size),
                        /* on error */ sail_destroy_variant(variant_local));

    *variant = variant_local;

    return SAIL_OK;
}

/*
 * Public functions.
 */
sail_status_t sail_alloc_variant(struct sail_variant** variant)
{
    SAIL_CHECK_PTR(variant);

    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_variant), &ptr));
    *variant = ptr;

    (*variant)->type  = SAIL_VARIANT_TYPE_INVALID;
    (*variant)->value = NULL;
    (*variant)->size  = 0;

    return SAIL_OK;
}

void sail_destroy_variant(struct sail_variant* variant)
{
    if (variant == NULL)
    {
        return;
    }

    sail_free(variant->value);
    sail_free(variant);
}

sail_status_t sail_set_variant_bool(struct sail_variant* variant, bool value)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_BOOL, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_char(struct sail_variant* variant, char value)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_CHAR, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_unsigned_char(struct sail_variant* variant, unsigned char value)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_UNSIGNED_CHAR, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_short(struct sail_variant* variant, short value)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_SHORT, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_unsigned_short(struct sail_variant* variant, unsigned short value)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_UNSIGNED_SHORT, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_int(struct sail_variant* variant, int value)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_INT, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_unsigned_int(struct sail_variant* variant, unsigned int value)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_UNSIGNED_INT, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_long(struct sail_variant* variant, long value)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_LONG, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_unsigned_long(struct sail_variant* variant, unsigned long value)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_UNSIGNED_LONG, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_long_long(struct sail_variant* variant, long long value)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_LONG_LONG, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_unsigned_long_long(struct sail_variant* variant, unsigned long long value)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_UNSIGNED_LONG_LONG, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_float(struct sail_variant* variant, float value)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_FLOAT, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_double(struct sail_variant* variant, double value)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_DOUBLE, &value, sizeof(value)));

    return SAIL_OK;
}

sail_status_t sail_set_variant_string(struct sail_variant* variant, const char* value)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_STRING, value, strlen(value) + 1));

    return SAIL_OK;
}

sail_status_t sail_set_variant_shallow_string(struct sail_variant* variant, char* value)
{
    SAIL_CHECK_PTR(variant);

    sail_free(variant->value);

    variant->type  = SAIL_VARIANT_TYPE_STRING;
    variant->value = value;
    variant->size  = strlen(value) + 1;

    return SAIL_OK;
}

sail_status_t sail_set_variant_substring(struct sail_variant* variant, const char* value, size_t size)
{
    SAIL_TRY(set_variant_value_asymmetric(variant, SAIL_VARIANT_TYPE_STRING, value, size + 1, size));

    char* str = variant->value;
    str[size] = '\0';

    return SAIL_OK;
}

sail_status_t sail_set_variant_data(struct sail_variant* variant, const void* value, size_t size)
{
    SAIL_TRY(set_variant_value(variant, SAIL_VARIANT_TYPE_DATA, value, size));

    return SAIL_OK;
}

sail_status_t sail_set_variant_shallow_data(struct sail_variant* variant, void* value, size_t size)
{
    SAIL_CHECK_PTR(variant);

    sail_free(variant->value);

    variant->type  = SAIL_VARIANT_TYPE_DATA;
    variant->value = value;
    variant->size  = size;

    return SAIL_OK;
}

bool sail_variant_to_bool(const struct sail_variant* variant)
{
    if (variant == NULL || variant->value == NULL)
    {
        return false;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_BOOL:
        return *(bool*)(variant->value);
    case SAIL_VARIANT_TYPE_STRING:
    {
        const char* str = (char*)(variant->value);
        if (str == NULL)
        {
            return false;
        }
        if (strcmp(str, "true") == 0 || strcmp(str, "1") == 0 || strcmp(str, "yes") == 0
            || strcmp(str, "TRUE") == 0 || strcmp(str, "YES") == 0)
        {
            return true;
        }
        return false;
    }
    default:
        return false;
    }
}

char sail_variant_to_char(const struct sail_variant* variant)
{
    if (variant == NULL || variant->value == NULL)
    {
        return 0;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_CHAR:
        return *(char*)(variant->value);
    case SAIL_VARIANT_TYPE_INT:
        return (char)*(int*)(variant->value);
    case SAIL_VARIANT_TYPE_UNSIGNED_INT:
        return (char)*(unsigned int*)(variant->value);
    case SAIL_VARIANT_TYPE_FLOAT:
        return (char)*(float*)(variant->value);
    case SAIL_VARIANT_TYPE_DOUBLE:
        return (char)*(double*)(variant->value);
    case SAIL_VARIANT_TYPE_STRING:
    {
        const char* str = (char*)(variant->value);
        if (str == NULL || str[0] == '\0')
        {
            return 0;
        }
        return str[0];
    }
    default:
        return 0;
    }
}

unsigned char sail_variant_to_unsigned_char(const struct sail_variant* variant)
{
    if (variant == NULL || variant->value == NULL)
    {
        return 0;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_UNSIGNED_CHAR:
        return *(unsigned char*)(variant->value);
    case SAIL_VARIANT_TYPE_CHAR:
    {
        char val = *(char*)(variant->value);
        return (val < 0) ? 0 : (unsigned char)val;
    }
    case SAIL_VARIANT_TYPE_INT:
    {
        int val = *(int*)(variant->value);
        return (val < 0) ? 0 : (val > 255) ? 255 : (unsigned char)val;
    }
    case SAIL_VARIANT_TYPE_UNSIGNED_INT:
    {
        unsigned int val = *(unsigned int*)(variant->value);
        return (val > 255) ? 255 : (unsigned char)val;
    }
    case SAIL_VARIANT_TYPE_FLOAT:
    {
        float val = *(float*)(variant->value);
        if (val < 0.0f)
        {
            return 0;
        }
        if (val > 255.0f)
        {
            return 255;
        }
        return (unsigned char)val;
    }
    case SAIL_VARIANT_TYPE_DOUBLE:
    {
        double val = *(double*)(variant->value);
        if (val < 0.0)
        {
            return 0;
        }
        if (val > 255.0)
        {
            return 255;
        }
        return (unsigned char)val;
    }
    case SAIL_VARIANT_TYPE_STRING:
    {
        const char* str = (char*)(variant->value);
        if (str == NULL || str[0] == '\0')
        {
            return 0;
        }
        char* endptr;
        unsigned long val = strtoul(str, &endptr, 10);
        if (endptr != str && *endptr == '\0')
        {
            return (val > 255) ? 255 : (unsigned char)val;
        }
        return 0;
    }
    default:
        return 0;
    }
}

short sail_variant_to_short(const struct sail_variant* variant)
{
    if (variant == NULL || variant->value == NULL)
    {
        return 0;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_SHORT:
        return *(short*)(variant->value);
    case SAIL_VARIANT_TYPE_INT:
        return (short)*(int*)(variant->value);
    case SAIL_VARIANT_TYPE_UNSIGNED_INT:
        return (short)*(unsigned int*)(variant->value);
    case SAIL_VARIANT_TYPE_FLOAT:
        return (short)*(float*)(variant->value);
    case SAIL_VARIANT_TYPE_DOUBLE:
        return (short)*(double*)(variant->value);
    case SAIL_VARIANT_TYPE_STRING:
    {
        const char* str = (char*)(variant->value);
        if (str == NULL)
        {
            return 0;
        }
        char* endptr;
        long val = strtol(str, &endptr, 10);
        if (endptr != str)
        {
            return (short)val;
        }
        return 0;
    }
    default:
        return 0;
    }
}

unsigned short sail_variant_to_unsigned_short(const struct sail_variant* variant)
{
    if (variant == NULL || variant->value == NULL)
    {
        return 0;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_UNSIGNED_SHORT:
        return *(unsigned short*)(variant->value);
    case SAIL_VARIANT_TYPE_SHORT:
    {
        short val = *(short*)(variant->value);
        return (val < 0) ? 0 : (unsigned short)val;
    }
    case SAIL_VARIANT_TYPE_INT:
    {
        int val = *(int*)(variant->value);
        return (val < 0) ? 0 : (unsigned short)val;
    }
    case SAIL_VARIANT_TYPE_UNSIGNED_INT:
        return (unsigned short)*(unsigned int*)(variant->value);
    case SAIL_VARIANT_TYPE_FLOAT:
    {
        float val = *(float*)(variant->value);
        return (val < 0.0f) ? 0 : (unsigned short)val;
    }
    case SAIL_VARIANT_TYPE_DOUBLE:
    {
        double val = *(double*)(variant->value);
        return (val < 0.0) ? 0 : (unsigned short)val;
    }
    case SAIL_VARIANT_TYPE_STRING:
    {
        const char* str = (char*)(variant->value);
        if (str == NULL)
        {
            return 0;
        }
        char* endptr;
        unsigned long val = strtoul(str, &endptr, 10);
        if (endptr != str && *endptr == '\0')
        {
            return (unsigned short)val;
        }
        return 0;
    }
    default:
        return 0;
    }
}

int sail_variant_to_int(const struct sail_variant* variant)
{
    if (variant == NULL || variant->value == NULL)
    {
        return 0;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_INT:
        return *(int*)(variant->value);
    case SAIL_VARIANT_TYPE_UNSIGNED_INT:
        return (int)*(unsigned int*)(variant->value);
    case SAIL_VARIANT_TYPE_FLOAT:
        return (int)*(float*)(variant->value);
    case SAIL_VARIANT_TYPE_DOUBLE:
        return (int)*(double*)(variant->value);
    case SAIL_VARIANT_TYPE_STRING:
    {
        const char* str = (char*)(variant->value);
        if (str == NULL)
        {
            return 0;
        }
        char* endptr;
        long val = strtol(str, &endptr, 10);
        if (endptr != str)
        {
            return (int)val;
        }
        return 0;
    }
    default:
        return 0;
    }
}

unsigned int sail_variant_to_unsigned_int(const struct sail_variant* variant)
{
    if (variant == NULL || variant->value == NULL)
    {
        return 0;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_INT:
    {
        int val = *(int*)(variant->value);
        return (val < 0) ? 0 : (unsigned int)val;
    }
    case SAIL_VARIANT_TYPE_UNSIGNED_INT:
        return *(unsigned int*)(variant->value);
    case SAIL_VARIANT_TYPE_FLOAT:
    {
        float val = *(float*)(variant->value);
        return (val < 0.0f) ? 0 : (unsigned int)val;
    }
    case SAIL_VARIANT_TYPE_DOUBLE:
    {
        double val = *(double*)(variant->value);
        return (val < 0.0) ? 0 : (unsigned int)val;
    }
    case SAIL_VARIANT_TYPE_STRING:
    {
        const char* str = (char*)(variant->value);
        if (str == NULL)
        {
            return 0;
        }
        char* endptr;
        unsigned long val = strtoul(str, &endptr, 10);
        if (endptr != str && *endptr == '\0')
        {
            return (unsigned int)val;
        }
        return 0;
    }
    default:
        return 0;
    }
}

long sail_variant_to_long(const struct sail_variant* variant)
{
    if (variant == NULL || variant->value == NULL)
    {
        return 0;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_LONG:
        return *(long*)(variant->value);
    case SAIL_VARIANT_TYPE_INT:
        return (long)*(int*)(variant->value);
    case SAIL_VARIANT_TYPE_UNSIGNED_INT:
        return (long)*(unsigned int*)(variant->value);
    case SAIL_VARIANT_TYPE_FLOAT:
        return (long)*(float*)(variant->value);
    case SAIL_VARIANT_TYPE_DOUBLE:
        return (long)*(double*)(variant->value);
    case SAIL_VARIANT_TYPE_STRING:
    {
        const char* str = (char*)(variant->value);
        if (str == NULL)
        {
            return 0;
        }
        char* endptr;
        long val = strtol(str, &endptr, 10);
        if (endptr != str)
        {
            return val;
        }
        return 0;
    }
    default:
        return 0;
    }
}

unsigned long sail_variant_to_unsigned_long(const struct sail_variant* variant)
{
    if (variant == NULL || variant->value == NULL)
    {
        return 0;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_UNSIGNED_LONG:
        return *(unsigned long*)(variant->value);
    case SAIL_VARIANT_TYPE_LONG:
    {
        long val = *(long*)(variant->value);
        return (val < 0) ? 0 : (unsigned long)val;
    }
    case SAIL_VARIANT_TYPE_INT:
    {
        int val = *(int*)(variant->value);
        return (val < 0) ? 0 : (unsigned long)val;
    }
    case SAIL_VARIANT_TYPE_UNSIGNED_INT:
        return (unsigned long)*(unsigned int*)(variant->value);
    case SAIL_VARIANT_TYPE_FLOAT:
    {
        float val = *(float*)(variant->value);
        return (val < 0.0f) ? 0 : (unsigned long)val;
    }
    case SAIL_VARIANT_TYPE_DOUBLE:
    {
        double val = *(double*)(variant->value);
        return (val < 0.0) ? 0 : (unsigned long)val;
    }
    case SAIL_VARIANT_TYPE_STRING:
    {
        const char* str = (char*)(variant->value);
        if (str == NULL)
        {
            return 0;
        }
        char* endptr;
        unsigned long val = strtoul(str, &endptr, 10);
        if (endptr != str && *endptr == '\0')
        {
            return val;
        }
        return 0;
    }
    default:
        return 0;
    }
}

long long sail_variant_to_long_long(const struct sail_variant* variant)
{
    if (variant == NULL || variant->value == NULL)
    {
        return 0;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_LONG_LONG:
        return *(long long*)(variant->value);
    case SAIL_VARIANT_TYPE_LONG:
        return (long long)*(long*)(variant->value);
    case SAIL_VARIANT_TYPE_INT:
        return (long long)*(int*)(variant->value);
    case SAIL_VARIANT_TYPE_UNSIGNED_INT:
        return (long long)*(unsigned int*)(variant->value);
    case SAIL_VARIANT_TYPE_FLOAT:
        return (long long)*(float*)(variant->value);
    case SAIL_VARIANT_TYPE_DOUBLE:
        return (long long)*(double*)(variant->value);
    case SAIL_VARIANT_TYPE_STRING:
    {
        const char* str = (char*)(variant->value);
        if (str == NULL)
        {
            return 0;
        }
        char* endptr;
        long long val = strtoll(str, &endptr, 10);
        if (endptr != str)
        {
            return val;
        }
        return 0;
    }
    default:
        return 0;
    }
}

unsigned long long sail_variant_to_unsigned_long_long(const struct sail_variant* variant)
{
    if (variant == NULL || variant->value == NULL)
    {
        return 0;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_UNSIGNED_LONG_LONG:
        return *(unsigned long long*)(variant->value);
    case SAIL_VARIANT_TYPE_LONG_LONG:
    {
        long long val = *(long long*)(variant->value);
        return (val < 0) ? 0 : (unsigned long long)val;
    }
    case SAIL_VARIANT_TYPE_LONG:
    {
        long val = *(long*)(variant->value);
        return (val < 0) ? 0 : (unsigned long long)val;
    }
    case SAIL_VARIANT_TYPE_INT:
    {
        int val = *(int*)(variant->value);
        return (val < 0) ? 0 : (unsigned long long)val;
    }
    case SAIL_VARIANT_TYPE_UNSIGNED_INT:
        return (unsigned long long)*(unsigned int*)(variant->value);
    case SAIL_VARIANT_TYPE_FLOAT:
    {
        float val = *(float*)(variant->value);
        return (val < 0.0f) ? 0 : (unsigned long long)val;
    }
    case SAIL_VARIANT_TYPE_DOUBLE:
    {
        double val = *(double*)(variant->value);
        return (val < 0.0) ? 0 : (unsigned long long)val;
    }
    case SAIL_VARIANT_TYPE_STRING:
    {
        const char* str = (char*)(variant->value);
        if (str == NULL)
        {
            return 0;
        }
        char* endptr;
        unsigned long long val = strtoull(str, &endptr, 10);
        if (endptr != str && *endptr == '\0')
        {
            return val;
        }
        return 0;
    }
    default:
        return 0;
    }
}

float sail_variant_to_float(const struct sail_variant* variant)
{
    if (variant == NULL || variant->value == NULL)
    {
        return 0.0f;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_INT:
        return (float)*(int*)(variant->value);
    case SAIL_VARIANT_TYPE_UNSIGNED_INT:
        return (float)*(unsigned int*)(variant->value);
    case SAIL_VARIANT_TYPE_FLOAT:
        return *(float*)(variant->value);
    case SAIL_VARIANT_TYPE_DOUBLE:
        return (float)*(double*)(variant->value);
    case SAIL_VARIANT_TYPE_STRING:
    {
        const char* str = (char*)(variant->value);
        if (str == NULL)
        {
            return 0.0f;
        }
        char* endptr;
        double val = strtod(str, &endptr);
        if (endptr != str && *endptr == '\0')
        {
            return (float)val;
        }
        return 0.0f;
    }
    default:
        return 0.0f;
    }
}

double sail_variant_to_double(const struct sail_variant* variant)
{
    if (variant == NULL || variant->value == NULL)
    {
        return 0.0;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_INT:
        return (double)*(int*)(variant->value);
    case SAIL_VARIANT_TYPE_UNSIGNED_INT:
        return (double)*(unsigned int*)(variant->value);
    case SAIL_VARIANT_TYPE_FLOAT:
        return (double)*(float*)(variant->value);
    case SAIL_VARIANT_TYPE_DOUBLE:
        return *(double*)(variant->value);
    case SAIL_VARIANT_TYPE_STRING:
    {
        const char* str = (char*)(variant->value);
        if (str == NULL)
        {
            return 0.0;
        }
        char* endptr;
        double val = strtod(str, &endptr);
        if (endptr != str && *endptr == '\0')
        {
            return val;
        }
        return 0.0;
    }
    default:
        return 0.0;
    }
}

char* sail_variant_to_string(const struct sail_variant* variant)
{
    return (char*)(variant->value);
}

void* sail_variant_to_data(const struct sail_variant* variant)
{
    return variant->value;
}

sail_status_t sail_check_variant_valid(const struct sail_variant* variant)
{
    SAIL_CHECK_PTR(variant);

    if (variant->type == SAIL_VARIANT_TYPE_INVALID || variant->value == NULL || variant->size == 0)
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_VARIANT);
    }

    return SAIL_OK;
}

sail_status_t sail_copy_variant(const struct sail_variant* source, struct sail_variant** target)
{
    SAIL_CHECK_PTR(source);
    SAIL_CHECK_PTR(target);

    SAIL_TRY(alloc_variant(source->type, source->value, source->size, target));

    return SAIL_OK;
}

bool sail_equal_variants(const struct sail_variant* variant1, const struct sail_variant* variant2)
{
    SAIL_TRY_OR_EXECUTE(sail_check_variant_valid(variant1),
                        /* on error */ return false);
    SAIL_TRY_OR_EXECUTE(sail_check_variant_valid(variant2),
                        /* on error */ return false);

    if (variant1->type != variant2->type || variant1->size != variant2->size)
    {
        return false;
    }
    else
    {
        return memcmp(variant1->value, variant2->value, variant1->size) == 0;
    }
}

int sail_printf_variant(const struct sail_variant* variant)
{
    return sail_fprintf_variant(variant, stdout);
}

int sail_fprintf_variant(const struct sail_variant* variant, FILE* f)
{
    if (variant == NULL)
    {
        return -1;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_BOOL: return fprintf(f, "%s", sail_variant_to_bool(variant) ? "true" : "false");
    case SAIL_VARIANT_TYPE_CHAR: return fprintf(f, "%d", sail_variant_to_char(variant));
    case SAIL_VARIANT_TYPE_UNSIGNED_CHAR: return fprintf(f, "%u", sail_variant_to_unsigned_char(variant));
    case SAIL_VARIANT_TYPE_SHORT: return fprintf(f, "%d", sail_variant_to_short(variant));
    case SAIL_VARIANT_TYPE_UNSIGNED_SHORT: return fprintf(f, "%u", sail_variant_to_unsigned_short(variant));
    case SAIL_VARIANT_TYPE_INT: return fprintf(f, "%d", sail_variant_to_int(variant));
    case SAIL_VARIANT_TYPE_UNSIGNED_INT: return fprintf(f, "%u", sail_variant_to_unsigned_int(variant));
    case SAIL_VARIANT_TYPE_LONG: return fprintf(f, "%ld", sail_variant_to_long(variant));
    case SAIL_VARIANT_TYPE_UNSIGNED_LONG: return fprintf(f, "%lu", sail_variant_to_unsigned_long(variant));
    case SAIL_VARIANT_TYPE_LONG_LONG: return fprintf(f, "%lld", sail_variant_to_long_long(variant));
    case SAIL_VARIANT_TYPE_UNSIGNED_LONG_LONG: return fprintf(f, "%llu", sail_variant_to_unsigned_long_long(variant));
    case SAIL_VARIANT_TYPE_FLOAT: return fprintf(f, "%f", sail_variant_to_float(variant));
    case SAIL_VARIANT_TYPE_DOUBLE: return fprintf(f, "%f", sail_variant_to_double(variant));
    case SAIL_VARIANT_TYPE_STRING: return fprintf(f, "%s", sail_variant_to_string(variant));
    case SAIL_VARIANT_TYPE_DATA: return fprintf(f, "<binary data, %u byte(s)>", (unsigned)variant->size);
    case SAIL_VARIANT_TYPE_INVALID: return fprintf(f, "<invalid value>");
    }

    return 0;
}

int sail_snprintf_variant(const struct sail_variant* variant, char* str, size_t str_size)
{
    if (variant == NULL)
    {
        return -1;
    }

    switch (variant->type)
    {
    case SAIL_VARIANT_TYPE_BOOL: return snprintf(str, str_size, "%s", sail_variant_to_bool(variant) ? "true" : "false");
    case SAIL_VARIANT_TYPE_CHAR: return snprintf(str, str_size, "%d", sail_variant_to_char(variant));
    case SAIL_VARIANT_TYPE_UNSIGNED_CHAR: return snprintf(str, str_size, "%u", sail_variant_to_unsigned_char(variant));
    case SAIL_VARIANT_TYPE_SHORT: return snprintf(str, str_size, "%d", sail_variant_to_short(variant));
    case SAIL_VARIANT_TYPE_UNSIGNED_SHORT:
        return snprintf(str, str_size, "%u", sail_variant_to_unsigned_short(variant));
    case SAIL_VARIANT_TYPE_INT: return snprintf(str, str_size, "%d", sail_variant_to_int(variant));
    case SAIL_VARIANT_TYPE_UNSIGNED_INT: return snprintf(str, str_size, "%u", sail_variant_to_unsigned_int(variant));
    case SAIL_VARIANT_TYPE_LONG: return snprintf(str, str_size, "%ld", sail_variant_to_long(variant));
    case SAIL_VARIANT_TYPE_UNSIGNED_LONG: return snprintf(str, str_size, "%lu", sail_variant_to_unsigned_long(variant));
    case SAIL_VARIANT_TYPE_LONG_LONG: return snprintf(str, str_size, "%lld", sail_variant_to_long_long(variant));
    case SAIL_VARIANT_TYPE_UNSIGNED_LONG_LONG: return snprintf(str, str_size, "%llu", sail_variant_to_unsigned_long_long(variant));
    case SAIL_VARIANT_TYPE_FLOAT: return snprintf(str, str_size, "%f", sail_variant_to_float(variant));
    case SAIL_VARIANT_TYPE_DOUBLE: return snprintf(str, str_size, "%f", sail_variant_to_double(variant));
    case SAIL_VARIANT_TYPE_STRING: return snprintf(str, str_size, "%s", sail_variant_to_string(variant));
    case SAIL_VARIANT_TYPE_DATA: return snprintf(str, str_size, "<binary data, %u byte(s)>", (unsigned)variant->size);
    case SAIL_VARIANT_TYPE_INVALID: return snprintf(str, str_size, "<invalid value>");
    }

    return 0;
}
