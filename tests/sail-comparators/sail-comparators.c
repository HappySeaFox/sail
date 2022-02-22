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

#include "sail-common.h"

#include "sail-comparators.h"

#include "munit.h"

sail_status_t sail_compare_resolutions(const struct sail_resolution *resolution1, const struct sail_resolution *resolution2) {

    munit_assert_not_null(resolution1);
    munit_assert_not_null(resolution2);

    munit_assert(resolution1 != resolution2);

    munit_assert(resolution1->unit == resolution2->unit);
    munit_assert(resolution1->x == resolution2->x);
    munit_assert(resolution1->y == resolution2->y);

    return SAIL_OK;
}

sail_status_t sail_compare_palettes(const struct sail_palette *palette1, const struct sail_palette *palette2) {

    munit_assert_not_null(palette1);
    munit_assert_not_null(palette2);

    munit_assert(palette1 != palette2);

    munit_assert(palette1->pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN);

    munit_assert(palette1->pixel_format == palette2->pixel_format);
    munit_assert(palette1->color_count == palette2->color_count);

    munit_assert_not_null(palette1->data);
    munit_assert_not_null(palette2->data);
    unsigned palette_size;
    munit_assert(sail_bytes_per_line(palette1->color_count, palette1->pixel_format, &palette_size) == SAIL_OK);
    munit_assert_memory_equal(palette_size, palette1->data, palette2->data);

    return SAIL_OK;
}

sail_status_t sail_compare_variants(const struct sail_variant *variant1, const struct sail_variant *variant2) {

    munit_assert_not_null(variant1);
    munit_assert_not_null(variant2);

    munit_assert(variant1 != variant2);

    munit_assert(variant1->value_type == variant2->value_type);
    munit_assert_memory_equal(variant1->value_size, variant1->value, variant2->value);
    munit_assert(variant1->value_size == variant2->value_size);

    return SAIL_OK;
}

sail_status_t sail_compare_meta_datas(const struct sail_meta_data *meta_data1, const struct sail_meta_data *meta_data2) {

    munit_assert_not_null(meta_data1);
    munit_assert_not_null(meta_data2);

    munit_assert(meta_data1 != meta_data2);

    munit_assert(meta_data1->key == meta_data2->key);

    if (meta_data1->key == SAIL_META_DATA_UNKNOWN) {
        munit_assert_not_null(meta_data1->key_unknown);
        munit_assert_not_null(meta_data2->key_unknown);
        munit_assert_string_equal(meta_data1->key_unknown, meta_data2->key_unknown);
    }

    munit_assert(sail_compare_variants(meta_data1->value, meta_data2->value) == SAIL_OK);

    return SAIL_OK;
}

sail_status_t sail_compare_meta_data_nodes(const struct sail_meta_data_node *meta_data_node1, const struct sail_meta_data_node *meta_data_node2) {

    munit_assert_not_null(meta_data_node1);
    munit_assert_not_null(meta_data_node2);

    munit_assert(meta_data_node1 != meta_data_node2);

    munit_assert(sail_compare_meta_datas(meta_data_node1->meta_data, meta_data_node2->meta_data) == SAIL_OK);

    return SAIL_OK;
}

sail_status_t sail_compare_meta_data_node_chains(const struct sail_meta_data_node *meta_data_node1, const struct sail_meta_data_node *meta_data_node2) {

    munit_assert_not_null(meta_data_node1);
    munit_assert_not_null(meta_data_node2);

    munit_assert(meta_data_node1 != meta_data_node2);

    while (meta_data_node1 != NULL) {
        munit_assert_not_null(meta_data_node2);

        munit_assert(sail_compare_meta_data_nodes(meta_data_node1, meta_data_node2) == SAIL_OK);

        meta_data_node1 = meta_data_node1->next;
        meta_data_node2 = meta_data_node2->next;
    }

    munit_assert_null(meta_data_node2);

    return SAIL_OK;
}

sail_status_t sail_compare_iccps(const struct sail_iccp *iccp1, const struct sail_iccp *iccp2) {

    munit_assert_not_null(iccp1);
    munit_assert_not_null(iccp2);

    munit_assert(iccp1 != iccp2);

    munit_assert(iccp1->data_length > 0);
    munit_assert(iccp1->data_length == iccp2->data_length);
    munit_assert_memory_equal(iccp1->data_length, iccp1->data, iccp2->data);

    return SAIL_OK;
}

sail_status_t sail_compare_source_images(const struct sail_source_image *source_image1, const struct sail_source_image *source_image2) {

    munit_assert_not_null(source_image1);
    munit_assert_not_null(source_image2);

    munit_assert(source_image1 != source_image2);

    munit_assert(source_image1->pixel_format == source_image2->pixel_format);
    munit_assert(source_image1->properties == source_image2->properties);
    munit_assert(source_image1->compression == source_image2->compression);

    return SAIL_OK;
}

sail_status_t sail_compare_images(const struct sail_image *image1, const struct sail_image *image2) {

    munit_assert_not_null(image1);
    munit_assert_not_null(image2);

    munit_assert(image1 != image2);

    munit_assert(image1->width > 0);
    munit_assert(image1->width == image2->width);
    munit_assert(image1->height > 0);
    munit_assert(image1->height == image2->height);
    munit_assert(image1->bytes_per_line > 0);
    munit_assert(image1->bytes_per_line == image2->bytes_per_line);

    munit_assert_not_null(image1->pixels);
    munit_assert_not_null(image2->pixels);
    const unsigned pixels_size = image1->height * image1->bytes_per_line;
    munit_assert_memory_equal(pixels_size, image1->pixels, image2->pixels);

    if (image1->resolution == NULL) {
        munit_assert_null(image2->resolution);
    } else {
        munit_assert(sail_compare_resolutions(image1->resolution, image2->resolution) == SAIL_OK);
    }

    munit_assert(image1->pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN);
    munit_assert(image1->pixel_format == image2->pixel_format);

    munit_assert(image1->delay == image2->delay);

    if (image1->palette == NULL) {
        munit_assert_null(image2->palette);
    } else {
        munit_assert(sail_compare_palettes(image1->palette, image2->palette) == SAIL_OK);
    }

    if (image1->meta_data_node == NULL) {
        munit_assert_null(image2->meta_data_node);
    } else {
        munit_assert(sail_compare_meta_data_node_chains(image1->meta_data_node, image2->meta_data_node) == SAIL_OK);
    }

    if (image1->iccp == NULL) {
        munit_assert_null(image2->iccp);
    } else {
        munit_assert(sail_compare_iccps(image1->iccp, image2->iccp) == SAIL_OK);
    }

    munit_assert(image1->properties == image2->properties);

    if (image1->source_image == NULL) {
        munit_assert_null(image2->source_image);
    } else {
        munit_assert(sail_compare_source_images(image1->source_image, image2->source_image) == SAIL_OK);
    }

    return SAIL_OK;
}
