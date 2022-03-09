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

#ifndef SAIL_COMPARATORS_H
#define SAIL_COMPARATORS_H

#include "error.h"
#include "export.h"

SAIL_EXPORT sail_status_t sail_test_compare_resolutions(const struct sail_resolution *resolution1, const struct sail_resolution *resolution2);

SAIL_EXPORT sail_status_t sail_test_compare_palettes(const struct sail_palette *palette1, const struct sail_palette *palette2);

SAIL_EXPORT sail_status_t sail_test_compare_variants(const struct sail_variant *variant1, const struct sail_variant *variant2);

SAIL_EXPORT sail_status_t sail_test_compare_hash_maps(const struct sail_hash_map *hash_map1, const struct sail_hash_map *hash_map2);

SAIL_EXPORT sail_status_t sail_test_compare_meta_datas(const struct sail_meta_data *meta_data1, const struct sail_meta_data *meta_data2);

SAIL_EXPORT sail_status_t sail_test_compare_meta_data_nodes(const struct sail_meta_data_node *meta_data_node1, const struct sail_meta_data_node *meta_data_node2);

SAIL_EXPORT sail_status_t sail_test_compare_meta_data_node_chains(const struct sail_meta_data_node *meta_data_node1, const struct sail_meta_data_node *meta_data_node2);

SAIL_EXPORT sail_status_t sail_test_compare_iccps(const struct sail_iccp *iccp1, const struct sail_iccp *iccp2);

SAIL_EXPORT sail_status_t sail_test_compare_source_images(const struct sail_source_image *source_image1, const struct sail_source_image *source_image2);

SAIL_EXPORT sail_status_t sail_test_compare_images(const struct sail_image *image1, const struct sail_image *image2);

#endif
