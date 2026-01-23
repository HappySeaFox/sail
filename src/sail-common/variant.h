/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2022 Dmitry Baryshev

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
#include <stddef.h> /* size_t */
#include <stdio.h>  /* FILE */

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Type of variant data.
 */
enum SailVariantType
{
    SAIL_VARIANT_TYPE_BOOL,
    SAIL_VARIANT_TYPE_CHAR,
    SAIL_VARIANT_TYPE_UNSIGNED_CHAR,
    SAIL_VARIANT_TYPE_SHORT,
    SAIL_VARIANT_TYPE_UNSIGNED_SHORT,
    SAIL_VARIANT_TYPE_INT,
    SAIL_VARIANT_TYPE_UNSIGNED_INT,
    SAIL_VARIANT_TYPE_LONG,
    SAIL_VARIANT_TYPE_UNSIGNED_LONG,
    SAIL_VARIANT_TYPE_FLOAT,
    SAIL_VARIANT_TYPE_DOUBLE,
    SAIL_VARIANT_TYPE_STRING,
    SAIL_VARIANT_TYPE_DATA,
    SAIL_VARIANT_TYPE_INVALID,

    /* Since 1.0.0. */
    SAIL_VARIANT_TYPE_LONG_LONG,
    SAIL_VARIANT_TYPE_UNSIGNED_LONG_LONG,
};

/*
 * Variant with limited possible data values.
 */
struct sail_variant
{
    /*
     * Value type.
     */
    enum SailVariantType type;

    /*
     * Pointer to the actual variant value.
     */
    void* value;

    /*
     * The size of the allocated memory for the value. For strings, it's strlen() + 1.
     */
    size_t size;
};

/*
 * Allocates a new invalid variant without any value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_variant(struct sail_variant** variant);

/*
 * Destroys the specified variant.
 */
SAIL_EXPORT void sail_destroy_variant(struct sail_variant* variant);

/*
 * Sets the specified boolean value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_bool(struct sail_variant* variant, bool value);

/*
 * Sets the specified char value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_char(struct sail_variant* variant, char value);

/*
 * Sets the specified unsigned char value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_unsigned_char(struct sail_variant* variant, unsigned char value);

/*
 * Sets the specified short value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_short(struct sail_variant* variant, short value);

/*
 * Sets the specified unsigned short value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_unsigned_short(struct sail_variant* variant, unsigned short value);

/*
 * Sets the specified int value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_int(struct sail_variant* variant, int value);

/*
 * Sets the specified unsigned int value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_unsigned_int(struct sail_variant* variant, unsigned int value);

/*
 * Sets the specified long value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_long(struct sail_variant* variant, long value);

/*
 * Sets the specified unsigned long value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_unsigned_long(struct sail_variant* variant, unsigned long value);

/*
 * Sets the specified long long value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_long_long(struct sail_variant* variant, long long value);

/*
 * Sets the specified unsigned long long value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_unsigned_long_long(struct sail_variant* variant, unsigned long long value);

/*
 * Sets the specified float value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_float(struct sail_variant* variant, float value);

/*
 * Sets the specified double value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_double(struct sail_variant* variant, double value);

/*
 * Sets a deep copy of the specified string as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_string(struct sail_variant* variant, const char* value);

/*
 * Sets a shallow copy of the specified string as a new variant value.
 * Transfers the ownership of the string to the constructed variant.
 * Do not free the string pointer. It will be freed in sail_destroy_variant().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_shallow_string(struct sail_variant* variant, char* value);

/*
 * Sets a deep copy of the specified substring as a new variant value.
 *
 * The size of the substring must not include a null character. Adds a null character to the end
 * of the constructed variant value, i.e. its final size is size + 1.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_substring(struct sail_variant* variant, const char* value, size_t size);

/*
 * Sets a deep copy of the specified data buffer as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_data(struct sail_variant* variant, const void* value, size_t size);

/*
 * Sets a shallow copy of the specified data buffer as a new variant value.
 * Transfers the ownership of the data pointer to the variant.
 * Do not free the data pointer. It will be freed in sail_destroy_variant().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_shallow_data(struct sail_variant* variant, void* value, size_t size);

/*
 * Sets the variant value using _Generic macro to automatically select
 * the appropriate function based on the value type.
 *
 * Example:
 *   sail_set_variant(variant, 42);       // int
 *   sail_set_variant(variant, 3.14);     // double
 *   sail_set_variant(variant, "hello");  // const char*
 *
 * Returns SAIL_OK on success.
 */
#define sail_set_variant_value(variant, value) \
    _Generic((value), \
        bool: sail_set_variant_bool, \
        char: sail_set_variant_char, \
        unsigned char: sail_set_variant_unsigned_char, \
        short: sail_set_variant_short, \
        unsigned short: sail_set_variant_unsigned_short, \
        int: sail_set_variant_int, \
        unsigned int: sail_set_variant_unsigned_int, \
        long: sail_set_variant_long, \
        unsigned long: sail_set_variant_unsigned_long, \
        long long: sail_set_variant_long_long, \
        unsigned long long: sail_set_variant_unsigned_long_long, \
        float: sail_set_variant_float, \
        double: sail_set_variant_double, \
        const char*: sail_set_variant_string, \
        char*: sail_set_variant_string \
    )(variant, value)

/*
 * Returns the variant value as a boolean. Supports conversion from bool and string types.
 * For string type, parses "true", "1", "yes" (case-insensitive) as true,
 * and "false", "0", "no" (case-insensitive) as false.
 * Returns false if conversion fails or variant is invalid.
 */
SAIL_EXPORT bool sail_variant_to_bool(const struct sail_variant* variant);

/*
 * Returns the variant value as a char. Supports conversion from char, int, unsigned int,
 * float, double, and string types. For string type, returns the first character.
 * Returns 0 if conversion fails or variant is invalid.
 */
SAIL_EXPORT char sail_variant_to_char(const struct sail_variant* variant);

/*
 * Returns the variant value as an unsigned char. Supports conversion from char, unsigned char,
 * int, unsigned int, float, double, and string types. For string type, parses the string as a decimal number
 * using strtoul(). Negative numeric values are clamped to 0. For string type, negative values are parsed
 * as unsigned (may result in large numbers). Values are clamped to [0, 255]. Returns 0 if conversion fails or variant is invalid.
 */
SAIL_EXPORT unsigned char sail_variant_to_unsigned_char(const struct sail_variant* variant);

/*
 * Returns the variant value as a short. Supports conversion from short, int, unsigned int,
 * float, double, and string types. For string type, parses the string as a decimal number.
 * Returns 0 if conversion fails or variant is invalid.
 */
SAIL_EXPORT short sail_variant_to_short(const struct sail_variant* variant);

/*
 * Returns the variant value as an unsigned short. Supports conversion from short, unsigned short,
 * int, unsigned int, float, double, and string types. For string type, parses the string as a decimal number
 * using strtoul(). Negative numeric values are clamped to 0. For string type, negative values are parsed
 * as unsigned (may result in large numbers). Returns 0 if conversion fails or variant is invalid.
 */
SAIL_EXPORT unsigned short sail_variant_to_unsigned_short(const struct sail_variant* variant);

/*
 * Returns the variant value as an integer. Supports conversion from int, unsigned int,
 * float, double, and string types. For string type, parses the string as a decimal number.
 * Returns 0 if conversion fails or variant is invalid.
 */
SAIL_EXPORT int sail_variant_to_int(const struct sail_variant* variant);

/*
 * Returns the variant value as an unsigned int. Supports conversion from int, unsigned int,
 * float, double, and string types. For string type, parses the string as a decimal number
 * using strtoul(). Negative numeric values are clamped to 0. For string type, negative values are parsed
 * as unsigned (may result in large numbers). Returns 0 if conversion fails or variant is invalid.
 */
SAIL_EXPORT unsigned int sail_variant_to_unsigned_int(const struct sail_variant* variant);

/*
 * Returns the variant value as a long. Supports conversion from long, int, unsigned int,
 * float, double, and string types. For string type, parses the string as a decimal number.
 * Returns 0 if conversion fails or variant is invalid.
 */
SAIL_EXPORT long sail_variant_to_long(const struct sail_variant* variant);

/*
 * Returns the variant value as an unsigned long. Supports conversion from long, unsigned long,
 * int, unsigned int, float, double, and string types. For string type, parses the string as a decimal number
 * using strtoul(). Negative numeric values are clamped to 0. For string type, negative values are parsed
 * as unsigned (may result in large numbers). Returns 0 if conversion fails or variant is invalid.
 */
SAIL_EXPORT unsigned long sail_variant_to_unsigned_long(const struct sail_variant* variant);

/*
 * Returns the variant value as a long long. Supports conversion from long long, long, int, unsigned int,
 * float, double, and string types. For string type, parses the string as a decimal number.
 * Returns 0 if conversion fails or variant is invalid.
 */
SAIL_EXPORT long long sail_variant_to_long_long(const struct sail_variant* variant);

/*
 * Returns the variant value as an unsigned long long. Supports conversion from unsigned long long, long long,
 * long, int, unsigned int, float, double, and string types. For string type, parses the string as a decimal number
 * using strtoull(). Negative numeric values are clamped to 0. For string type, negative values are parsed
 * as unsigned (may result in large numbers). Returns 0 if conversion fails or variant is invalid.
 */
SAIL_EXPORT unsigned long long sail_variant_to_unsigned_long_long(const struct sail_variant* variant);

/*
 * Returns the variant value as a float. Supports conversion from int, unsigned int,
 * float, double, and string types. For string type, parses the string as a floating-point number.
 * Returns 0.0f if conversion fails or variant is invalid.
 */
SAIL_EXPORT float sail_variant_to_float(const struct sail_variant* variant);

/*
 * Returns the variant value as a double. Supports conversion from int, unsigned int,
 * float, double, and string types. For string type, parses the string as a floating-point number.
 * Returns 0.0 if conversion fails or variant is invalid.
 */
SAIL_EXPORT double sail_variant_to_double(const struct sail_variant* variant);

/*
 * Returns the variant value as a string. Behavior is undefined if the variant is invalid.
 * Effectively, it casts the value pointer to char*, and returns the resulting pointer.
 */
SAIL_EXPORT char* sail_variant_to_string(const struct sail_variant* variant);

/*
 * Returns the variant value as a binary data. Behavior is undefined if the variant is invalid.
 */
SAIL_EXPORT void* sail_variant_to_data(const struct sail_variant* variant);

/*
 * Checks the variant is not NULL and holds a valid value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_check_variant_valid(const struct sail_variant* variant);

/*
 * Makes a deep copy of the specified variant.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_variant(const struct sail_variant* source, struct sail_variant** target);

/*
 * Returns true if both the variants are valid and contain equal values.
 */
SAIL_EXPORT bool sail_equal_variants(const struct sail_variant* variant1, const struct sail_variant* variant2);

/*
 * Calls printf() to print the value of the variant. If the variant is NULL,
 * doesn't print anything and returns -1.
 *
 * Returns the result of printf().
 */
SAIL_EXPORT int sail_printf_variant(const struct sail_variant* variant);

/*
 * Calls fprintf() to print the value of the variant into the file.
 * If the variant is NULL, doesn't print anything and returns -1.
 *
 * Returns the result of fprintf().
 */
SAIL_EXPORT int sail_fprintf_variant(const struct sail_variant* variant, FILE* f);

/*
 * Calls snprintf() to put the value of the variant into the string.
 * If the variant is NULL, doesn't print anything and returns -1.
 *
 * Returns the result of snprintf().
 */
SAIL_EXPORT int sail_snprintf_variant(const struct sail_variant* variant, char* str, size_t str_size);

/* extern "C" */
#ifdef __cplusplus
}
#endif
