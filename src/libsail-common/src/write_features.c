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

int sail_alloc_write_features(struct sail_write_features **write_features) {

    SAIL_TRY(sail_malloc(write_features, sizeof(struct sail_write_features)));

    (*write_features)->pixel_formats_mapping_node = NULL;
    (*write_features)->features                   = 0;
    (*write_features)->properties                 = 0;
    (*write_features)->interlaced_passes          = 0;
    (*write_features)->compression_types          = NULL;
    (*write_features)->compression_types_length   = 0;
    (*write_features)->preferred_compression_type = 0;
    (*write_features)->compression_min            = 0;
    (*write_features)->compression_max            = 0;
    (*write_features)->compression_default        = 0;

    return 0;
}

void sail_destroy_write_features(struct sail_write_features *write_features) {

    if (write_features == NULL) {
        return;
    }

    sail_destroy_pixel_formats_mapping_node_chain(write_features->pixel_formats_mapping_node);

    free(write_features->compression_types);
    free(write_features);
}
