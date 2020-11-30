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

sail_status_t sail_alloc_write_features(struct sail_write_features **write_features) {

    SAIL_CHECK_WRITE_FEATURES_PTR(write_features);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_write_features), &ptr));
    *write_features = ptr;

    (*write_features)->pixel_formats_mapping_node = NULL;
    (*write_features)->features                   = 0;
    (*write_features)->properties                 = 0;
    (*write_features)->interlaced_passes          = 0;
    (*write_features)->compressions               = NULL;
    (*write_features)->compressions_length        = 0;
    (*write_features)->default_compression        = SAIL_COMPRESSION_UNSUPPORTED;
    (*write_features)->compression_level_min      = 0;
    (*write_features)->compression_level_max      = 0;
    (*write_features)->compression_level_default  = 0;
    (*write_features)->compression_level_step     = 0;

    return SAIL_OK;
}

void sail_destroy_write_features(struct sail_write_features *write_features) {

    if (write_features == NULL) {
        return;
    }

    sail_destroy_pixel_formats_mapping_node_chain(write_features->pixel_formats_mapping_node);

    sail_free(write_features->compressions);
    sail_free(write_features);
}
