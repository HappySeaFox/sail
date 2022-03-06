/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#ifndef SAIL_VARIANT_H
#define SAIL_VARIANT_H

#include <stdbool.h>
#include <stddef.h>

#ifdef SAIL_BUILD
    #include "common.h"
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/common.h>
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Type of variant data.
 */
enum SailVariantType {

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
};

/*
 * Variant with limited possible data values.
 */
struct sail_variant {

    /*
     * Value type.
     */
    enum SailVariantType type;

    /*
     * Pointer to the actual variant value.
     */
    void *value;

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
SAIL_EXPORT sail_status_t sail_alloc_variant(struct sail_variant **variant);

/*
 * Destroys the specified variant.
 */
SAIL_EXPORT void sail_destroy_variant(struct sail_variant *variant);

/*
 * Sets the specified boolean value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_bool(struct sail_variant *variant, bool value);

/*
 * Sets the specified char value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_char(struct sail_variant *variant, char value);

/*
 * Sets the specified unsigned char value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_unsigned_char(struct sail_variant *variant, unsigned char value);

/*
 * Sets the specified short value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_short(struct sail_variant *variant, short value);

/*
 * Sets the specified unsigned short value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_unsigned_short(struct sail_variant *variant, unsigned short value);

/*
 * Sets the specified int value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_int(struct sail_variant *variant, int value);

/*
 * Sets the specified unsigned int value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_unsigned_int(struct sail_variant *variant, unsigned int value);

/*
 * Sets the specified long value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_long(struct sail_variant *variant, long value);

/*
 * Sets the specified unsigned long value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_unsigned_long(struct sail_variant *variant, unsigned long value);

/*
 * Sets the specified float value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_float(struct sail_variant *variant, float value);

/*
 * Sets the specified double value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_double(struct sail_variant *variant, double value);

/*
 * Sets a deep copy of the specified string as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_string(struct sail_variant *variant, const char *value);

/*
 * Sets a shallow copy of the specified string as a new variant value.
 * Transfers the ownership of the string to the constructed variant.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_adopted_string(struct sail_variant *variant, char *value);

/*
 * Sets a deep copy of the specified substring as a new variant value.
 *
 * The size of the substring must not include a null character. Adds a null character to the end
 * of the constructed variant value, i.e. its final size is size + 1.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_substring(struct sail_variant *variant, const char *value, size_t size);

/*
 * Sets a deep copy of the specified data buffer as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_data(struct sail_variant *variant, const void *value, size_t size);

/*
 * Sets a shallow copy of the specified data buffer as a new variant value.
 * Transfers the ownership of the data pointer to the variant.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_adopted_data(struct sail_variant *variant, void *value, size_t size);

/*
 * Returns the variant value as a boolean. Behavior is undefined if the variant is invalid.
 * Effectively, it casts the value pointer to bool*, and then dereferences the resulting pointer.
 */
SAIL_EXPORT bool sail_variant_to_bool(const struct sail_variant *variant);

/*
 * Returns the variant value as a char. Behavior is undefined if the variant is invalid.
 * Effectively, it casts the value pointer to char*, and then dereferences the resulting pointer.
 */
SAIL_EXPORT char sail_variant_to_char(const struct sail_variant *variant);

/*
 * Returns the variant value as an unsigned char. Behavior is undefined if the variant is invalid.
 * Effectively, it casts the value pointer to unsigned char*, and then dereferences the resulting pointer.
 */
SAIL_EXPORT unsigned char sail_variant_to_unsigned_char(const struct sail_variant *variant);

/*
 * Returns the variant value as a short. Behavior is undefined if the variant is invalid.
 * Effectively, it casts the value pointer to short*, and then dereferences the resulting pointer.
 */
SAIL_EXPORT short sail_variant_to_short(const struct sail_variant *variant);

/*
 * Returns the variant value as an unsigned short. Behavior is undefined if the variant is invalid.
 * Effectively, it casts the value pointer to unsigned short*, and then dereferences the resulting pointer.
 */
SAIL_EXPORT unsigned short sail_variant_to_unsigned_short(const struct sail_variant *variant);

/*
 * Returns the variant value as an integer. Behavior is undefined if the variant is invalid.
 * Effectively, it casts the value pointer to int*, and then dereferences the resulting pointer.
 */
SAIL_EXPORT int sail_variant_to_int(const struct sail_variant *variant);

/*
 * Returns the variant value as an unsigned int. Behavior is undefined if the variant is invalid.
 * Effectively, it casts the value pointer to unsigned int*, and then dereferences the resulting pointer.
 */
SAIL_EXPORT unsigned int sail_variant_to_unsigned_int(const struct sail_variant *variant);

/*
 * Returns the variant value as a long. Behavior is undefined if the variant is invalid.
 * Effectively, it casts the value pointer to long*, and then dereferences the resulting pointer.
 */
SAIL_EXPORT long sail_variant_to_long(const struct sail_variant *variant);

/*
 * Returns the variant value as an unsigned long. Behavior is undefined if the variant is invalid.
 * Effectively, it casts the value pointer to unsigned long*, and then dereferences the resulting pointer.
 */
SAIL_EXPORT unsigned long sail_variant_to_unsigned_long(const struct sail_variant *variant);

/*
 * Returns the variant value as a float. Behavior is undefined if the variant is invalid.
 * Effectively, it casts the value pointer to float*, and then dereferences the resulting pointer.
 */
SAIL_EXPORT float sail_variant_to_float(const struct sail_variant *variant);

/*
 * Returns the variant value as a double. Behavior is undefined if the variant is invalid.
 * Effectively, it casts the value pointer to double*, and then dereferences the resulting pointer.
 */
SAIL_EXPORT double sail_variant_to_double(const struct sail_variant *variant);

/*
 * Returns the variant value as a string. Behavior is undefined if the variant is invalid.
 * Effectively, it casts the value pointer to char*, and returns the resulting pointer.
 */
SAIL_EXPORT char* sail_variant_to_string(const struct sail_variant *variant);

/*
 * Returns the variant value as a binary data. Behavior is undefined if the variant is invalid.
 */
SAIL_EXPORT void* sail_variant_to_data(const struct sail_variant *variant);

/*
 * Checks the variant is not NULL and holds a valid value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_check_variant_valid(const struct sail_variant *variant);

/*
 * Makes a deep copy of the specified variant.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_variant(const struct sail_variant *source, struct sail_variant **target);

/*
 * Returns true if both the variants are valid and contain equal values.
 */
SAIL_EXPORT bool sail_equal_variants(const struct sail_variant *variant1, const struct sail_variant *variant2);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
