/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

#include "sail-common.h"

#include "munit.h"

static MunitResult test_vector_alloc_destroy(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_vector *vector = NULL;

    munit_assert(sail_alloc_vector(0, NULL, &vector) == SAIL_OK);
    munit_assert_not_null(vector);
    munit_assert(sail_vector_size(vector) == 0);
    sail_destroy_vector(vector);

    return MUNIT_OK;
}

static MunitResult test_vector_clear(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_vector *vector = NULL;

    munit_assert(sail_alloc_vector(5, NULL, &vector) == SAIL_OK);
    munit_assert_not_null(vector);
    munit_assert(sail_vector_size(vector) == 0);

    int *item;
    munit_assert(sail_malloc(sizeof(int), &item) == SAIL_OK);
    *item = 5;

    munit_assert(sail_push_vector(vector, item) == SAIL_OK);
    sail_clear_vector(vector);
    sail_destroy_vector(vector);

    return MUNIT_OK;
}

static MunitResult test_vector_push_pop(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_vector *vector;

    munit_assert(sail_alloc_vector(0, NULL, &vector) == SAIL_OK);

    for (size_t i = 0; i < 3; i++) {
        int *item;
        munit_assert(sail_malloc(sizeof(int), &item) == SAIL_OK);
        *item = 5;

        munit_assert(sail_push_vector(vector, item) == SAIL_OK);
        munit_assert(sail_vector_size(vector) == i+1);
    }

    sail_pop_vector(vector);
    munit_assert(sail_vector_size(vector) == 2);
    sail_pop_vector(vector);
    munit_assert(sail_vector_size(vector) == 1);

    sail_destroy_vector(vector);

    return MUNIT_OK;
}

static void free_image_item(void *item) {

    sail_destroy_image(item);
}

static MunitResult test_vector_item_destroy(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    struct sail_vector *vector;

    const size_t capacity = 10;
    munit_assert(sail_alloc_vector(capacity, free_image_item, &vector) == SAIL_OK);

    for (size_t i = 0; i < capacity; i++) {
        struct sail_image *image;
        munit_assert(sail_alloc_image(&image) == SAIL_OK);
        munit_assert(sail_push_vector(vector, image) == SAIL_OK);
    }

    sail_destroy_vector(vector);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/alloc-destroy", test_vector_alloc_destroy, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/clear",         test_vector_clear,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/push-pop",      test_vector_push_pop,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/item-destroy",  test_vector_item_destroy,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/vector",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
