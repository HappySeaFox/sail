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
    munit_assert(variant->type == SAIL_VARIANT_TYPE_INVALID);
    munit_assert_null(variant->value);
    munit_assert(variant->size == 0);

    sail_destroy_variant(variant);

    return MUNIT_OK;
}

static MunitResult test_copy(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const int reference_value = 64;

    struct sail_variant *variant;
    munit_assert(sail_alloc_variant(&variant) == SAIL_OK);
    munit_assert(sail_set_variant_int(variant, reference_value) == SAIL_OK);

    struct sail_variant *variant_copy = NULL;
    munit_assert(sail_copy_variant(variant, &variant_copy) == SAIL_OK);

    munit_assert_not_null(variant_copy);
    munit_assert(variant_copy->type == SAIL_VARIANT_TYPE_INT);
    munit_assert(sail_variant_to_int(variant_copy) == reference_value);
    munit_assert(variant_copy->size == sizeof(reference_value));

    sail_destroy_variant(variant_copy);
    sail_destroy_variant(variant);

    return MUNIT_OK;
}

#define TEST_VARIANT_FROM_VALUE(VALUE_TYPE, VALUE, SETTER, VARIANT_TYPE, ACCESSOR) \
do {                                                       \
    VALUE_TYPE s = VALUE;                                  \
                                                           \
    struct sail_variant *variant;                          \
    munit_assert(sail_alloc_variant(&variant) == SAIL_OK); \
    munit_assert(SETTER(variant, s) == SAIL_OK);           \
                                                           \
    munit_assert(variant->type == VARIANT_TYPE);           \
    munit_assert(ACCESSOR(variant) == s);                  \
    munit_assert(variant->size == sizeof(VALUE_TYPE));     \
                                                           \
    sail_destroy_variant(variant);                         \
} while(0)

static MunitResult test_from_value(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    TEST_VARIANT_FROM_VALUE(bool, true, sail_set_variant_bool, SAIL_VARIANT_TYPE_BOOL, sail_variant_to_bool);

    TEST_VARIANT_FROM_VALUE(char,          'a', sail_set_variant_char,          SAIL_VARIANT_TYPE_CHAR,          sail_variant_to_char);
    TEST_VARIANT_FROM_VALUE(unsigned char, 'b', sail_set_variant_unsigned_char, SAIL_VARIANT_TYPE_UNSIGNED_CHAR, sail_variant_to_unsigned_char);

    TEST_VARIANT_FROM_VALUE(short,          2110, sail_set_variant_short,          SAIL_VARIANT_TYPE_SHORT,          sail_variant_to_short);
    TEST_VARIANT_FROM_VALUE(unsigned short, 2110, sail_set_variant_unsigned_short, SAIL_VARIANT_TYPE_UNSIGNED_SHORT, sail_variant_to_unsigned_short);

    TEST_VARIANT_FROM_VALUE(int,          0xFFFF5, sail_set_variant_int,          SAIL_VARIANT_TYPE_INT,          sail_variant_to_int);
    TEST_VARIANT_FROM_VALUE(unsigned int, 0xFFFF5, sail_set_variant_unsigned_int, SAIL_VARIANT_TYPE_UNSIGNED_INT, sail_variant_to_unsigned_int);

    TEST_VARIANT_FROM_VALUE(long,          0xFFFF6, sail_set_variant_long,          SAIL_VARIANT_TYPE_LONG,          sail_variant_to_long);
    TEST_VARIANT_FROM_VALUE(unsigned long, 0xFFFF6, sail_set_variant_unsigned_long, SAIL_VARIANT_TYPE_UNSIGNED_LONG, sail_variant_to_unsigned_long);

    TEST_VARIANT_FROM_VALUE(float,         160.f,   sail_set_variant_float,  SAIL_VARIANT_TYPE_FLOAT,  sail_variant_to_float);
    TEST_VARIANT_FROM_VALUE(double,        29555.0, sail_set_variant_double, SAIL_VARIANT_TYPE_DOUBLE, sail_variant_to_double);

    return MUNIT_OK;
}

#define TEST_VARIANT_FROM_STRING(VALUE, SETTER, VARIANT_TYPE, ACCESSOR) \
do {                                                       \
    void *ptr = (void *)VALUE;                             \
    struct sail_variant *variant;                          \
    munit_assert(sail_alloc_variant(&variant) == SAIL_OK); \
    munit_assert(SETTER(variant, ptr) == SAIL_OK);         \
                                                           \
    munit_assert(variant->type == VARIANT_TYPE);           \
    munit_assert(strcmp(ACCESSOR(variant), ptr) == 0);     \
    munit_assert(variant->size == strlen(ptr) + 1);        \
                                                           \
    sail_destroy_variant(variant);                         \
} while(0)

static MunitResult test_from_string(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    TEST_VARIANT_FROM_STRING("abc", sail_set_variant_string, SAIL_VARIANT_TYPE_STRING, sail_variant_to_string);

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

    TEST_VARIANT_FROM_STRING(str, sail_set_variant_adopted_string, SAIL_VARIANT_TYPE_STRING, sail_variant_to_string);

    return MUNIT_OK;
}

#define TEST_VARIANT_FROM_DATA(VALUE, VALUE_SIZE, SETTER, VARIANT_TYPE, ACCESSOR) \
do {                                                               \
    void *ptr = (void *)VALUE;                                     \
    struct sail_variant *variant;                                  \
    munit_assert(sail_alloc_variant(&variant) == SAIL_OK);         \
    munit_assert(SETTER(variant, ptr, VALUE_SIZE) == SAIL_OK);     \
                                                                   \
    munit_assert(variant->type == VARIANT_TYPE);                   \
    munit_assert(memcmp(ACCESSOR(variant), ptr, VALUE_SIZE) == 0); \
    munit_assert(variant->size == VALUE_SIZE);                     \
                                                                   \
    sail_destroy_variant(variant);                                 \
} while(0)

static MunitResult test_from_data(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    TEST_VARIANT_FROM_DATA("abc", 3, sail_set_variant_data, SAIL_VARIANT_TYPE_DATA, sail_variant_to_data);

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

    TEST_VARIANT_FROM_DATA(str, 3, sail_set_variant_adopted_data, SAIL_VARIANT_TYPE_DATA, sail_variant_to_data);

    return MUNIT_OK;
}

static MunitResult test_set(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    struct sail_variant *variant;
    munit_assert(sail_alloc_variant(&variant) == SAIL_OK);

    munit_assert(sail_set_variant_bool(variant, true) == SAIL_OK);
    munit_assert(sail_variant_to_bool(variant) == true);

    munit_assert(sail_set_variant_char(variant, 6) == SAIL_OK);
    munit_assert(sail_variant_to_char(variant) == 6);

    munit_assert(sail_set_variant_unsigned_char(variant, 7) == SAIL_OK);
    munit_assert(sail_variant_to_unsigned_char(variant) == 7);

    munit_assert(sail_set_variant_short(variant, 19) == SAIL_OK);
    munit_assert(sail_variant_to_short(variant) == 19);

    munit_assert(sail_set_variant_unsigned_short(variant, 29) == SAIL_OK);
    munit_assert(sail_variant_to_unsigned_short(variant) == 29);

    munit_assert(sail_set_variant_int(variant, 0xFFFF9) == SAIL_OK);
    munit_assert(sail_variant_to_int(variant) == 0xFFFF9);

    munit_assert(sail_set_variant_unsigned_int(variant, 0xFFFFFF9) == SAIL_OK);
    munit_assert(sail_variant_to_unsigned_int(variant) == 0xFFFFFF9);

    munit_assert(sail_set_variant_long(variant, 0xFFFF9) == SAIL_OK);
    munit_assert(sail_variant_to_long(variant) == 0xFFFF9);

    munit_assert(sail_set_variant_unsigned_long(variant, 0xFFFFFF9) == SAIL_OK);
    munit_assert(sail_variant_to_unsigned_long(variant) == 0xFFFFFF9);

    munit_assert(sail_set_variant_string(variant, "abc") == SAIL_OK);
    munit_assert(strcmp(sail_variant_to_string(variant), "abc") == 0);

    munit_assert(sail_set_variant_substring(variant, "abc", 2) == SAIL_OK);
    munit_assert(strcmp(sail_variant_to_string(variant), "ab") == 0);

    munit_assert(sail_set_variant_data(variant, "abc", 4) == SAIL_OK);
    munit_assert(strcmp((char *)sail_variant_to_data(variant), "abc") == 0);

    sail_destroy_variant(variant);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/alloc",       test_alloc,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/copy",        test_copy,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-value",  test_from_value,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-string", test_from_string, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/from-data",   test_from_data,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/set",         test_set,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/variant",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
