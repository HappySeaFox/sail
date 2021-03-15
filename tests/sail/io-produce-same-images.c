/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020-2021 Dmitry Baryshev

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
#include <string.h>

#include "sail-common.h"
#include "sail.h"

#include "munit.h"

#include "test-images.h"

static MunitResult test_io_produce_same_images(const MunitParameter params[], void *user_data) {
    (void)user_data;

    const char *path = munit_parameters_get(params, "path");

    struct sail_image *image_file;
    munit_assert(sail_read_file(path, &image_file) == SAIL_OK);
    munit_assert_not_null(image_file);

    void *buffer;
    unsigned buffer_length;
    munit_assert(sail_read_file_contents(path, &buffer, &buffer_length) == SAIL_OK);
    munit_assert_not_null(buffer);
    munit_assert(buffer_length > 0);

    struct sail_image *image_mem;
    munit_assert(sail_read_mem(buffer, buffer_length, &image_mem) == SAIL_OK);
    munit_assert_not_null(image_mem);

    munit_assert(image_file->width > 0);
    munit_assert(image_file->width == image_mem->width);
    munit_assert(image_file->height > 0);
    munit_assert(image_file->height == image_mem->height);
    munit_assert(image_file->bytes_per_line > 0);
    munit_assert(image_file->bytes_per_line == image_mem->bytes_per_line);

    munit_assert_not_null(image_file->pixels);
    munit_assert_not_null(image_mem->pixels);
    const unsigned pixels_size = image_file->height * image_file->bytes_per_line;
    munit_assert(memcmp(image_file->pixels, image_mem->pixels, pixels_size) == 0);

    if (image_file->resolution == NULL) {
        munit_assert_null(image_mem->resolution);
    } else {
        munit_assert_not_null(image_mem->resolution);

        munit_assert(image_file->resolution->unit == image_mem->resolution->unit);
        munit_assert(image_file->resolution->x == image_mem->resolution->x);
        munit_assert(image_file->resolution->y == image_mem->resolution->y);
    }

    munit_assert(image_file->pixel_format != SAIL_PIXEL_FORMAT_AUTO);
    munit_assert(image_file->pixel_format != SAIL_PIXEL_FORMAT_SOURCE);
    munit_assert(image_file->pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN);
    munit_assert(image_file->pixel_format == image_mem->pixel_format);

    munit_assert(image_file->animated == image_mem->animated);
    munit_assert(image_file->delay == image_mem->delay);

    if (image_file->palette == NULL) {
        munit_assert_null(image_mem->palette);
    } else {
        munit_assert_not_null(image_mem->palette);

        munit_assert(image_file->palette->pixel_format != SAIL_PIXEL_FORMAT_AUTO);
        munit_assert(image_file->palette->pixel_format != SAIL_PIXEL_FORMAT_SOURCE);
        munit_assert(image_file->palette->pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN);

        munit_assert(image_file->palette->color_count == image_mem->palette->color_count);

        munit_assert_not_null(image_file->palette->data);
        munit_assert_not_null(image_mem->palette->data);
        unsigned palette_size;
        munit_assert(sail_bytes_per_line(image_file->palette->color_count, image_file->palette->pixel_format, &palette_size) == SAIL_OK);
        munit_assert(memcmp(image_file->palette->data, image_mem->palette->data, palette_size) == 0);
    }

    if (image_file->meta_data_node == NULL) {
        munit_assert_null(image_mem->meta_data_node);
    } else {
        munit_assert_not_null(image_mem->meta_data_node);

        const struct sail_meta_data_node *file_node = image_file->meta_data_node;
        const struct sail_meta_data_node *mem_node = image_mem->meta_data_node;

        while (file_node != NULL) {
            munit_assert_not_null(mem_node);

            munit_assert(file_node->key == mem_node->key);

            if (file_node->key == SAIL_META_DATA_UNKNOWN) {
                munit_assert_not_null(file_node->key_unknown);
                munit_assert_not_null(mem_node->key_unknown);
                munit_assert_string_equal(file_node->key_unknown, mem_node->key_unknown);
            }

            munit_assert(file_node->value_type == mem_node->value_type);

            if (file_node->value_type == SAIL_META_DATA_TYPE_STRING) {
                munit_assert_not_null(file_node->value_string);
                munit_assert_not_null(mem_node->value_string);
                munit_assert_string_equal(file_node->value_string, mem_node->value_string);
            } else if (file_node->value_type == SAIL_META_DATA_TYPE_DATA) {
                munit_assert(file_node->value_data_length > 0);
                munit_assert(file_node->value_data_length == mem_node->value_data_length);
                munit_assert(memcmp(file_node->value_data, mem_node->value_data, file_node->value_data_length) == 0);
            } else {
                munit_assert(false);
            }

            file_node = file_node->next;
            mem_node = mem_node->next;
        }

        munit_assert_null(mem_node);
    }

    if (image_file->iccp == NULL) {
        munit_assert_null(image_mem->iccp);
    } else {
        munit_assert_not_null(image_file->iccp->data);
        munit_assert(image_file->iccp->data_length == image_mem->iccp->data_length);
        munit_assert(memcmp(image_file->iccp->data, image_mem->iccp->data, image_file->iccp->data_length) == 0);
    }

    munit_assert(image_file->properties == image_mem->properties);

    if (image_file->source_image == NULL) {
        munit_assert_null(image_mem->source_image);
    } else {
        munit_assert(image_file->source_image->pixel_format == image_mem->source_image->pixel_format);
        munit_assert(image_file->source_image->properties == image_mem->source_image->properties);
        munit_assert(image_file->source_image->compression == image_mem->source_image->compression);
    }

    sail_free(buffer);
    sail_destroy_image(image_mem);
    sail_destroy_image(image_file);

    return MUNIT_OK;
}

static MunitParameterEnum test_params[] = {
    { (char *)"path", (char **)SAIL_TEST_IMAGES },
    { NULL, NULL },
};

static MunitTest test_suite_tests[] = {
    { (char *)"/io-produce-same-images", test_io_produce_same_images, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/io-produce-same-images",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
