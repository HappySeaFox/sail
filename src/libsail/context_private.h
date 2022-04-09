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

#ifndef SAIL_CONTEXT_PRIVATE_H
#define SAIL_CONTEXT_PRIVATE_H

#include <stdbool.h>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_codec_bundle_node;

/*
 * Context is a main entry point to start working with SAIL. It enumerates codec info objects which could be
 * used later in loading and saving operations.
 */
struct sail_context {

    /* Context is already initialized. */
    bool initialized;

    /* Linked list of found codec info objects. */
    struct sail_codec_bundle_node *codec_bundle_node;
};

typedef struct sail_context sail_context_t;

SAIL_HIDDEN sail_status_t destroy_global_context(void);

SAIL_HIDDEN sail_status_t fetch_global_context_guarded(struct sail_context **context);

SAIL_HIDDEN sail_status_t fetch_global_context_unsafe(struct sail_context **context);

SAIL_HIDDEN sail_status_t fetch_global_context_guarded_with_flags(struct sail_context **context, int flags);

SAIL_HIDDEN sail_status_t fetch_global_context_unsafe_with_flags(struct sail_context **context, int flags);

SAIL_HIDDEN sail_status_t sail_unload_codecs_private(void);

SAIL_HIDDEN sail_status_t lock_context(void);

SAIL_HIDDEN sail_status_t unlock_context(void);

#endif
