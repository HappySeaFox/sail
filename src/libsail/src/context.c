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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"
#include "sail.h"

sail_status_t sail_init_with_flags(int flags) {

    struct sail_context *context;
    SAIL_TRY(current_tls_context_with_flags(&context, flags));

    return SAIL_OK;
}

void sail_finish(void) {

    SAIL_LOG_INFO("Finish");

    control_tls_context(/* context - not needed */ NULL, SAIL_CONTEXT_DESTROY);
}

sail_status_t sail_unload_codecs(void) {

    SAIL_LOG_DEBUG("Unloading cached codecs");

    struct sail_context *context;
    SAIL_TRY(current_tls_context(&context));

    struct sail_codec_info_node *node = context->codec_info_node;
    int counter = 0;

    while (node != NULL) {
        if (node->codec != NULL) {
            destroy_codec(node->codec);
            counter++;
        }

        node->codec = NULL;

        node = node->next;
    }

    SAIL_LOG_DEBUG("Unloaded codecs: %d", counter);

    return SAIL_OK;
}
