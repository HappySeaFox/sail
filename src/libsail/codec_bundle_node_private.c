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

#include "sail.h"

sail_status_t alloc_codec_bundle_node(struct sail_codec_bundle_node **codec_bundle_node) {

    SAIL_TRY(sail_private_alloc_linked_list_node((struct linked_list_node **)codec_bundle_node));

    return SAIL_OK;
}

void destroy_codec_bundle_node(struct sail_codec_bundle_node *codec_bundle_node) {

    sail_private_destroy_linked_list_node((struct linked_list_node *)codec_bundle_node,
                                          (linked_list_value_deallocator_t)&destroy_codec_bundle);
}

void destroy_codec_bundle_node_chain(struct sail_codec_bundle_node *codec_bundle_node) {

    sail_private_destroy_linked_list_node_chain((struct linked_list_node *)codec_bundle_node,
                                                (linked_list_value_deallocator_t)&destroy_codec_bundle);
}
