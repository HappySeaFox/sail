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

sail_status_t sail_alloc_save_features(struct sail_save_features **save_features) {

    SAIL_CHECK_PTR(save_features);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_save_features), &ptr));
    *save_features = ptr;

    (*save_features)->pixel_formats             = NULL;
    (*save_features)->pixel_formats_length      = 0;
    (*save_features)->features                  = 0;
    (*save_features)->compressions              = NULL;
    (*save_features)->compressions_length       = 0;
    (*save_features)->default_compression       = SAIL_COMPRESSION_UNKNOWN;
    (*save_features)->compression_level_min     = 0;
    (*save_features)->compression_level_max     = 0;
    (*save_features)->compression_level_default = 0;
    (*save_features)->compression_level_step    = 0;
    (*save_features)->tuning                    = NULL;

    return SAIL_OK;
}

void sail_destroy_save_features(struct sail_save_features *save_features) {

    if (save_features == NULL) {
        return;
    }

    sail_free(save_features->pixel_formats);
    sail_free(save_features->compressions);
    sail_destroy_string_node_chain(save_features->tuning);
    sail_free(save_features);
}
