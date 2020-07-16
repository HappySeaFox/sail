/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#include <stdio.h>
#include <stdlib.h>

#include "sail-common.h"

sail_error_t sail_alloc_pixel_formats_mapping_node(struct sail_pixel_formats_mapping_node **node) {

    SAIL_TRY(sail_malloc(node, sizeof(struct sail_pixel_formats_mapping_node)));

    (*node)->input_pixel_format          = SAIL_PIXEL_FORMAT_UNKNOWN;
    (*node)->output_pixel_formats        = NULL;
    (*node)->output_pixel_formats_length = 0;
    (*node)->next                        = NULL;

    return 0;
}

void sail_destroy_pixel_formats_mapping_node(struct sail_pixel_formats_mapping_node *node) {

    if (node == NULL) {
        return;
    }

    free(node->output_pixel_formats);
    free(node);
}

void sail_destroy_pixel_formats_mapping_node_chain(struct sail_pixel_formats_mapping_node *node) {

    while (node != NULL) {
        struct sail_pixel_formats_mapping_node *node_next = node->next;

        sail_destroy_pixel_formats_mapping_node(node);

        node = node_next;
    }
}
