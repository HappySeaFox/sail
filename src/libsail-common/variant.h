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

#include <stddef.h>
#include <time.h>

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

    SAIL_VARIANT_TYPE_CHAR,
    SAIL_VARIANT_TYPE_UNSIGNED_CHAR,
    SAIL_VARIANT_TYPE_SHORT,
    SAIL_VARIANT_TYPE_UNSIGNED_SHORT,
    SAIL_VARIANT_TYPE_INT,
    SAIL_VARIANT_TYPE_UNSIGNED_INT,
    SAIL_VARIANT_TYPE_LONG,
    SAIL_VARIANT_TYPE_UNSIGNED_LONG,
    SAIL_VARIANT_TYPE_TIMESTAMP, /* Unix timestamp. */
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
    enum SailVariantType value_type;

    /*
     * Pointer to the actual variant value.
     */
    void *value;

    /*
     * The size of the allocated memory for the value. For strings, it's strlen() + 1.
     */
    size_t value_size;
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
 * Sets the specified Unix timestamp value as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_timestamp(struct sail_variant *variant, time_t value);

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
 * of the constructed variant value, i.e. its final size is value_size + 1.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_substring(struct sail_variant *variant, const char *value, size_t value_size);

/*
 * Sets a deep copy of the specified data buffer as a new variant value.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_data(struct sail_variant *variant, const void *value, size_t value_size);

/*
 * Sets a shallow copy of the specified data buffer as a new variant value.
 * Transfers the ownership of the data pointer to the variant.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_set_variant_adopted_data(struct sail_variant *variant, void *value, size_t value_size);

/*
 * Returns the variant value as a char. Behavior is undefined if the variant is invalid
 * or its type cannot be safely converted to char.
 */
SAIL_EXPORT char sail_variant_to_char(const struct sail_variant *variant);

/*
 * Returns the variant value as an unsigned char. Behavior is undefined if the variant is invalid
 * or its type cannot be safely converted to unsigned char.
 */
SAIL_EXPORT unsigned char sail_variant_to_unsigned_char(const struct sail_variant *variant);

/*
 * Returns the variant value as a short. Behavior is undefined if the variant is invalid
 * or its type cannot be safely converted to short.
 */
SAIL_EXPORT short sail_variant_to_short(const struct sail_variant *variant);

/*
 * Returns the variant value as an unsigned short. Behavior is undefined if the variant is invalid
 * or its type cannot be safely converted to unsigned short.
 */
SAIL_EXPORT unsigned short sail_variant_to_unsigned_short(const struct sail_variant *variant);

/*
 * Returns the variant value as an integer. Behavior is undefined if the variant is invalid
 * or its type cannot be safely converted to int.
 */
SAIL_EXPORT int sail_variant_to_int(const struct sail_variant *variant);

/*
 * Returns the variant value as an unsigned int. Behavior is undefined if the variant is invalid
 * or its type cannot be safely converted to unsigned int.
 */
SAIL_EXPORT unsigned int sail_variant_to_unsigned_int(const struct sail_variant *variant);

/*
 * Returns the variant value as a long. Behavior is undefined if the variant is invalid
 * or its type cannot be safely converted to long.
 */
SAIL_EXPORT long sail_variant_to_long(const struct sail_variant *variant);

/*
 * Returns the variant value as an unsigned long. Behavior is undefined if the variant is invalid
 * or its type cannot be safely converted to unsigned long.
 */
SAIL_EXPORT unsigned long sail_variant_to_unsigned_long(const struct sail_variant *variant);

/*
 * Returns the variant value as a Unix timestamp. Behavior is undefined if the variant is invalid
 * or its type cannot be safely converted to Unix timestamp.
 */
SAIL_EXPORT time_t sail_variant_to_timestamp(const struct sail_variant *variant);

/*
 * Returns the variant value as a string. Behavior is undefined if the variant is invalid
 * or its type cannot be safely converted to string.
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

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
