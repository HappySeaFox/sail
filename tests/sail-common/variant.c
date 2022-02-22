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

#include <stddef.h>
#include <string.h>

#include "sail-common.h"

#include "munit.h"

static MunitResult test_alloc(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    struct sail_variant *variant = NULL;
    munit_assert(sail_alloc_variant(&variant) == SAIL_OK);

    munit_assert_not_null(variant);
    munit_assert(variant->value_type == SAIL_VARIANT_TYPE_INVALID);
    munit_assert_null(variant->value);
    munit_assert(variant->value_size == 0);

    sail_destroy_variant(variant);

    return MUNIT_OK;
}

static MunitResult test_copy(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const int reference_value = 64;

    struct sail_variant *variant;
    munit_assert(sail_alloc_variant_from_int(reference_value, &variant) == SAIL_OK);

    struct sail_variant *variant_copy = NULL;
    munit_assert(sail_copy_variant(variant, &variant_copy) == SAIL_OK);

    munit_assert_not_null(variant_copy);
    munit_assert(variant_copy->value_type == SAIL_VARIANT_TYPE_INT);
    munit_assert(sail_variant_to_int(variant_copy) == reference_value);
    munit_assert(variant_copy->value_size == sizeof(reference_value));

    sail_destroy_variant(variant_copy);
    sail_destroy_variant(variant);

    return MUNIT_OK;
}

#define TEST_VARIANT_FROM_VALUE(VALUE_TYPE, VALUE, ALLOCATOR, VARIANT_TYPE, ACCESSOR) \
do {                                                         \
    VALUE_TYPE s = VALUE;                                    \
                                                             \
    struct sail_variant *variant;                            \
    munit_assert(ALLOCATOR(s, &variant) == SAIL_OK);         \
                                                             \
    munit_assert(variant->value_type == VARIANT_TYPE);       \
    munit_assert(ACCESSOR(variant) == s);                    \
    munit_assert(variant->value_size == sizeof(VALUE_TYPE)); \
                                                             \
    sail_destroy_variant(variant);                           \
} while(0)

static MunitResult test_from_value(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    TEST_VARIANT_FROM_VALUE(char,          'a', sail_alloc_variant_from_char,          SAIL_VARIANT_TYPE_CHAR,          sail_variant_to_char);
    TEST_VARIANT_FROM_VALUE(unsigned char, 'b', sail_alloc_variant_from_unsigned_char, SAIL_VARIANT_TYPE_UNSIGNED_CHAR, sail_variant_to_unsigned_char);

    TEST_VARIANT_FROM_VALUE(short,          2110, sail_alloc_variant_from_short,          SAIL_VARIANT_TYPE_SHORT,          sail_variant_to_short);
    TEST_VARIANT_FROM_VALUE(unsigned short, 2110, sail_alloc_variant_from_unsigned_short, SAIL_VARIANT_TYPE_UNSIGNED_SHORT, sail_variant_to_unsigned_short);

    TEST_VARIANT_FROM_VALUE(int,          0xFFFF5, sail_alloc_variant_from_int,          SAIL_VARIANT_TYPE_INT,          sail_variant_to_int);
    TEST_VARIANT_FROM_VALUE(unsigned int, 0xFFFF5, sail_alloc_variant_from_unsigned_int, SAIL_VARIANT_TYPE_UNSIGNED_INT, sail_variant_to_unsigned_int);

    TEST_VARIANT_FROM_VALUE(long,          0xFFFF6, sail_alloc_variant_from_long,          SAIL_VARIANT_TYPE_LONG,          sail_variant_to_long);
    TEST_VARIANT_FROM_VALUE(unsigned long, 0xFFFF6, sail_alloc_variant_from_unsigned_long, SAIL_VARIANT_TYPE_UNSIGNED_LONG, sail_variant_to_unsigned_long);

    time_t timestamp = time(NULL);
    TEST_VARIANT_FROM_VALUE(time_t, timestamp, sail_alloc_variant_from_timestamp, SAIL_VARIANT_TYPE_TIMESTAMP, sail_variant_to_timestamp);

    return MUNIT_OK;
}

#define TEST_VARIANT_FROM_STRING(VALUE, ALLOCATOR, VARIANT_TYPE, ACCESSOR) \
do {                                                      \
    void *ptr = (void *)VALUE;                            \
    struct sail_variant *variant;                         \
    munit_assert(ALLOCATOR(ptr, &variant) == SAIL_OK);    \
                                                          \
    munit_assert(variant->value_type == VARIANT_TYPE);    \
    munit_assert(strcmp(ACCESSOR(variant), ptr) == 0);    \
    munit_assert(variant->value_size == strlen(ptr) + 1); \
                                                          \
    sail_destroy_variant(variant);                        \
} while(0)

static MunitResult test_from_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    TEST_VARIANT_FROM_STRING("abc", sail_alloc_variant_from_string, SAIL_VARIANT_TYPE_STRING, sail_variant_to_string);

    const char *copy_from = "xyz";
    const size_t copy_size = strlen(copy_from) + 1;
    char *str;
    {
        void *ptr;
        munit_assert(sail_malloc(copy_size, &ptr) == SAIL_OK);
        str = ptr;
    }
#ifdef _MSC_VER
    strcpy_s(str, copy_size, "xyz");
#else
    strcpy(str, "xyz");
#endif

    TEST_VARIANT_FROM_STRING(str, sail_alloc_variant_from_adopted_string, SAIL_VARIANT_TYPE_STRING, sail_variant_to_string);

    return MUNIT_OK;
}

#define TEST_VARIANT_FROM_DATA(VALUE, VALUE_SIZE, ALLOCATOR, VARIANT_TYPE, ACCESSOR) \
do {                                                               \
    void *ptr = (void *)VALUE;                                     \
    struct sail_variant *variant;                                  \
    munit_assert(ALLOCATOR(ptr, VALUE_SIZE, &variant) == SAIL_OK); \
                                                                   \
    munit_assert(variant->value_type == VARIANT_TYPE);             \
    munit_assert(memcmp(ACCESSOR(variant), ptr, VALUE_SIZE) == 0); \
    munit_assert(variant->value_size == VALUE_SIZE);               \
                                                                   \
    sail_destroy_variant(variant);                                 \
} while(0)

static MunitResult test_from_data(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    TEST_VARIANT_FROM_DATA("abc", 3, sail_alloc_variant_from_data, SAIL_VARIANT_TYPE_DATA, sail_variant_to_data);

    const char *copy_from = "xyz";
    const size_t copy_size = strlen(copy_from) + 1;
    char *str;
    {
        void *ptr;
        munit_assert(sail_malloc(copy_size, &ptr) == SAIL_OK);
        str = ptr;
    }
#ifdef _MSC_VER
    strcpy_s(str, copy_size, "xyz");
#else
    strcpy(str, "xyz");
#endif

    TEST_VARIANT_FROM_DATA(str, 3, sail_alloc_variant_from_adopted_data, SAIL_VARIANT_TYPE_DATA, sail_variant_to_data);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/alloc",       test_alloc,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy",        test_copy,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-value",  test_from_value,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-string", test_from_string, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-data",   test_from_data,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/variant",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

long main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}